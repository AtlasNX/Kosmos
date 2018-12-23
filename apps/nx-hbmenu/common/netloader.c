#include "common.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include <sys/types.h>

#ifndef __WIN32__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#else

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
typedef uint32_t in_addr_t;

#undef DrawText

#endif


#include "netloader.h"

#define PING_ENABLED 1

#ifndef __SWITCH__
#include "switch/runtime/nxlink.h"
#endif

#define ZLIB_CHUNK (16 * 1024)
#define FILE_BUFFER_SIZE (128*1024)

static int netloader_listenfd = -1;
static int netloader_datafd   = -1;
#if PING_ENABLED
static int netloader_udpfd = -1;
#endif
static unsigned char in[ZLIB_CHUNK];
static unsigned char out[ZLIB_CHUNK];

static mtx_t netloader_mtx;

static menuEntry_s netloader_me;
static volatile bool netloader_initialized = 0;
static volatile bool netloader_exitflag = 0;
static volatile bool netloader_activated = 0, netloader_launchapp = 0;
static volatile size_t netloader_filelen, netloader_filetotal;
static volatile char netloader_errortext[1024];

static bool netloaderGetExit(void);

//---------------------------------------------------------------------------------
static void netloader_error(const char *func, int err) {
//---------------------------------------------------------------------------------
    if (!netloader_initialized || netloaderGetExit()) return;

    mtx_lock(&netloader_mtx);
    if (netloader_errortext[0] == 0) {
        memset((char*)netloader_errortext, 0, sizeof(netloader_errortext));
        snprintf((char*)netloader_errortext, sizeof(netloader_errortext)-1, "%s: err=%d\n %s\n", func, err, strerror(errno));
    }
    mtx_unlock(&netloader_mtx);
}

//---------------------------------------------------------------------------------
static void netloader_socket_error(const char *func) {
//---------------------------------------------------------------------------------
    int errcode;
#ifdef __WIN32__
    errcode = WSAGetLastError();
#else
    errcode = errno;
#endif
    netloader_error(func,errcode);
}

//---------------------------------------------------------------------------------
void shutdownSocket(int socket) {
//---------------------------------------------------------------------------------
#ifdef __WIN32__
    shutdown (socket, SD_SEND);
    closesocket (socket);
#else
    close(socket);
#endif
}

static const char DIRECTORY_THIS[] = ".";
static const char DIRECTORY_PARENT[] = "..";

//---------------------------------------------------------------------------------
static bool isDirectorySeparator(int c) {
//---------------------------------------------------------------------------------
    return c == DIRECTORY_SEPARATOR_CHAR;
}

//---------------------------------------------------------------------------------
static void sanitisePath(char *path) {
//---------------------------------------------------------------------------------
    char *tmpPath = strdup(path);
    tmpPath[0] = 0;

    char *dirStart = path;
    char *curPath = tmpPath;

#ifdef _WIN32
    while(dirStart[0]) {
        if (dirStart[0] == '/') dirStart[0] =DIRECTORY_SEPARATOR_CHAR;
        dirStart++;
    }
#endif

    dirStart = path;

    while(isDirectorySeparator(dirStart[0])) dirStart++;


    do {
        char *dirEnd = strchr(dirStart, DIRECTORY_SEPARATOR_CHAR);
        if (dirEnd) {
            dirEnd++;
            if(!strncmp(DIRECTORY_PARENT,dirStart,strlen(DIRECTORY_PARENT))) {
                /* move back one directory */
                size_t pathlen = strlen(tmpPath);
                if(tmpPath[pathlen-1] == DIRECTORY_SEPARATOR_CHAR) tmpPath[pathlen-1] = 0;
                char *prev = strrchr(tmpPath,DIRECTORY_SEPARATOR_CHAR);
                if (prev) {
                    curPath = prev + 1;
                } else {
                    curPath = tmpPath;
                }


                dirStart = dirEnd;
            } else if (!strncmp(DIRECTORY_THIS,dirStart,strlen(DIRECTORY_THIS))) {
                /* strip this entry */
                dirStart = dirEnd;
            } else {
                size_t dirSize = dirEnd - dirStart;
                strncpy(curPath,dirStart,dirSize);
                curPath[dirSize] = 0;
                curPath += dirSize;
                dirStart += dirSize;
            }
        } else {
            strcpy(curPath,dirStart);
            dirStart += strlen(dirStart);
        }
    } while(dirStart[0]);

    strcpy(path, tmpPath);
    free(tmpPath);
}

//---------------------------------------------------------------------------------
static int set_socket_nonblocking(int sock) {
//---------------------------------------------------------------------------------

#ifndef __WIN32__
    int flags = fcntl(sock, F_GETFL);

    if(flags == -1) return -1;

    int rc = fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    if(rc != 0) return -1;

#else
    u_long opt = 1;
    ioctlsocket(sock, FIONBIO, &opt);
#endif


    return 0;
}


//---------------------------------------------------------------------------------
static int recvall(int sock, void *buffer, int size, int flags) {
//---------------------------------------------------------------------------------
    int len, sizeleft = size;
    bool blockflag=0;

    while (sizeleft) {

        len = recv(sock,buffer,sizeleft,flags);

        if (len == 0) {
            size = 0;
            break;
        };

        if (len != -1) {
            sizeleft -=len;
            buffer +=len;
        } else {
#ifdef _WIN32
            int errcode = WSAGetLastError();
            if (errcode != WSAEWOULDBLOCK) {
                netloader_error("win socket error",errcode);
                break;
            }
            else {
                blockflag = 1;
            }
#else
            if ( errno != EWOULDBLOCK && errno != EAGAIN) {
                netloader_socket_error("recv");
                break;
            }
            else {
                blockflag = 1;
            }
#endif

            if (blockflag && netloaderGetExit()) return 0;
        }
    }
    return size;
}

//---------------------------------------------------------------------------------
static int sendall(int sock, void *buffer, int size, int flags) {
//---------------------------------------------------------------------------------
    int len, sizeleft = size;
    bool blockflag=0;

    while (sizeleft) {

        len = send(sock,buffer,sizeleft,flags);

        if (len == 0) {
            size = 0;
            break;
        };

        if (len != -1) {
            sizeleft -=len;
            buffer +=len;
        } else {
#ifdef _WIN32
            int errcode = WSAGetLastError();
            if (errcode != WSAEWOULDBLOCK) {
                netloader_error("win socket error",errcode);
                break;
            }
            else {
                blockflag = 1;
            }
#else
            if ( errno != EWOULDBLOCK && errno != EAGAIN) {
                netloader_socket_error("recv");
                break;
            }
            else {
                blockflag = 1;
            }
#endif

            if (blockflag && netloaderGetExit()) return 0;
        }
    }
    return size;
}

//---------------------------------------------------------------------------------
static int decompress(int sock, FILE *fh, size_t filesize) {
//---------------------------------------------------------------------------------
    int ret;
    unsigned have;
    z_stream strm;
    size_t chunksize;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        netloader_error("inflateInit failed.",ret);
        return ret;
    }

    size_t total = 0;
    /* decompress until deflate stream ends or end of file */
    do {
        if (netloaderGetExit()) {
            ret = Z_DATA_ERROR;
            break;
        }

        int len = recvall(sock, &chunksize, 4, 0);

        if (len != 4) {
            (void)inflateEnd(&strm);
            netloader_error("Error getting chunk size",len);
            return Z_DATA_ERROR;
        }

    strm.avail_in = recvall(sock,in,chunksize,0);

    if (strm.avail_in == 0) {
        (void)inflateEnd(&strm);
        netloader_error("remote closed socket.",0);
        return Z_DATA_ERROR;
    }

    strm.next_in = in;

    /* run inflate() on input until output buffer not full */
    do {
        strm.avail_out = ZLIB_CHUNK;
        strm.next_out = out;
        ret = inflate(&strm, Z_NO_FLUSH);

        switch (ret) {

            case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     /* and fall through */

            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
            case Z_STREAM_ERROR:
                (void)inflateEnd(&strm);
                netloader_error("inflate error",ret);
                return ret;
        }

        have = ZLIB_CHUNK - strm.avail_out;

        if (fwrite(out, 1, have, fh) != have || ferror(fh)) {
            (void)inflateEnd(&strm);
            netloader_error("file write error",0);
            return Z_ERRNO;
        }

        total += have;
        mtx_lock(&netloader_mtx);
        netloader_filetotal = total;
        mtx_unlock(&netloader_mtx);
        //printf("%zu (%zd%%)",total, (100 * total) / filesize);
    } while (strm.avail_out == 0);

    /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}



//---------------------------------------------------------------------------------
int loadnro(menuEntry_s *me, int sock, struct in_addr remote) {
//---------------------------------------------------------------------------------
    int len, namelen, filelen;
    char filename[PATH_MAX+1];
    len = recvall(sock, &namelen, 4, 0);

    if (len != 4) {
        netloader_error("Error getting name length", errno);
        return -1;
    }

    if (namelen >= sizeof(filename)-1) {
        netloader_error("Filename length is too large",errno);
        return -1;
    }

    len = recvall(sock, filename, namelen, 0);

    if (len != namelen) {
        netloader_error("Error getting filename", errno);
        return -1;
    }

    filename[namelen] = 0;

    len = recvall(sock, &filelen, 4, 0);

    if (len != 4) {
        netloader_error("Error getting file length",errno);
        return -1;
    }

    mtx_lock(&netloader_mtx);
    netloader_filelen = filelen;
    mtx_unlock(&netloader_mtx);

    int response = 0;

    sanitisePath(filename);

    snprintf(me->path, sizeof(me->path)-1, "%s%s%s", menuGetRootPath(), DIRECTORY_SEPARATOR,  filename);
    me->path[PATH_MAX] = 0;
    // make sure it's terminated
    me->path[PATH_MAX] = 0;

    argData_s* ad = &me->args;
    ad->dst = (char*)&ad->buf[1];
    ad->nxlink_host = remote;

    launchAddArg(ad, me->path);

#ifndef _WIN32
    int fd = open(me->path,O_CREAT|O_WRONLY, ACCESSPERMS);

    if (fd < 0) {
        response = -1;
        netloader_error("open", errno);
    } else {
        if (ftruncate(fd,filelen) == -1) {
            response = -2;
            netloader_error("ftruncate",errno);
        }
        close(fd);
    }
#endif

    FILE *file = NULL;

    if (response == 0) file = fopen(me->path,"wb");

    if(NULL == file) {
        perror("file");
        response = -1;
    }

    send(sock,(char *)&response,sizeof(response),0);

    if (response == 0 ) {

        //char *writebuffer=malloc(FILE_BUFFER_SIZE);
        //setvbuf(file,writebuffer,_IOFBF, FILE_BUFFER_SIZE);

        //printf("transferring %s\n%d bytes.\n", filename, filelen);

        if (decompress(sock,file,filelen)==Z_OK) {
            int netloaded_cmdlen = 0;
            len = sendall(sock,(char *)&response,sizeof(response),0);

            if (len != sizeof(response)) {
                netloader_error("Error sending response",errno);
                response = -1;
            }

            //printf("\ntransferring command line\n");

            if (response == 0 ) {
                len = recvall(sock,(char*)&netloaded_cmdlen,4,0);

                if (len != 4) {
                    netloader_error("Error getting netloaded_cmdlen",errno);
                    response = -1;
                }
            }

            if (response == 0 ) {
                if (netloaded_cmdlen > sizeof(me->args.buf)-1) netloaded_cmdlen = sizeof(me->args.buf)-1;

                len = recvall(sock,me->args.dst, netloaded_cmdlen,0);

                if (len != netloaded_cmdlen) {
                    netloader_error("Error getting args",errno);
                    response = -1;
                }
            }

            if (response == 0 ) {
                while(netloaded_cmdlen) {
                    size_t len = strlen(me->args.dst) + 1;
                    ad->dst += len;
                    ad->buf[0]++;
                    netloaded_cmdlen -= len;
                }
            }

        } else {
            response = -1;
        }

        //free(writebuffer);
        fflush(file);
        fclose(file);

        if (response == -1) unlink(me->path);
    }

    return response;
}

//---------------------------------------------------------------------------------
int netloader_activate(void) {
//---------------------------------------------------------------------------------
    struct sockaddr_in serv_addr;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(NXLINK_SERVER_PORT);

#if PING_ENABLED
    // create udp socket for broadcast ping
    netloader_udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (netloader_udpfd < 0)
    {
        netloader_socket_error("udp socket");
        return -1;
    }

    if(bind(netloader_udpfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        netloader_socket_error("bind udp socket");
        return -1;
    }

    if (set_socket_nonblocking(netloader_udpfd) == -1)
    {
        netloader_socket_error("listen fcntl");
        return -1;
    }
#endif
    // create listening socket on all addresses on NXLINK_SERVER_PORT

    netloader_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(netloader_listenfd < 0)
    {
        netloader_socket_error("socket");
        return -1;
    }

    int rc = bind(netloader_listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(rc != 0)
    {
        netloader_socket_error("bind");
        return -1;
    }

    if (set_socket_nonblocking(netloader_listenfd) == -1)
    {
        netloader_socket_error("listen fcntl");
        return -1;
    }

    rc = listen(netloader_listenfd, 10);
        if(rc != 0)
    {
        netloader_socket_error("listen");
        return -1;
    }

    return 0;
}


//---------------------------------------------------------------------------------
int netloader_deactivate(void) {
//---------------------------------------------------------------------------------
    // close all remaining sockets and allow mainloop to return to main menu
    if(netloader_listenfd >= 0)
    {
        shutdownSocket(netloader_listenfd);
        netloader_listenfd = -1;
    }

    if(netloader_datafd >= 0)
    {
        shutdownSocket(netloader_datafd);
        netloader_datafd = -1;
    }

#if PING_ENABLED
    if(netloader_udpfd >= 0)
    {
        shutdownSocket(netloader_udpfd);
        netloader_udpfd = -1;
    }
#endif

    return 0;
}

//---------------------------------------------------------------------------------
int netloader_loop(struct sockaddr_in *sa_remote) {
//---------------------------------------------------------------------------------

#if PING_ENABLED
    char recvbuf[256];
    socklen_t fromlen = sizeof(struct sockaddr_in);

    int len = recvfrom(netloader_udpfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*) sa_remote, &fromlen);

    if (len!=-1) {
        if (strncmp(recvbuf,"nxboot",strlen("nxboot")) == 0) {
            sa_remote->sin_family=AF_INET;
            sa_remote->sin_port=htons(NXLINK_CLIENT_PORT);
            sendto(netloader_udpfd, "bootnx", strlen("bootnx"), 0, (struct sockaddr*) sa_remote,sizeof(struct sockaddr_in));
        }
    }
#endif
    if(netloader_listenfd >= 0 && netloader_datafd < 0) {
        socklen_t addrlen = sizeof(struct sockaddr_in);
        netloader_datafd = accept(netloader_listenfd, (struct sockaddr*)sa_remote, &addrlen);
        if(netloader_datafd < 0)
        {

#ifdef _WIN32
            int errcode = WSAGetLastError();
            if (errcode != WSAEWOULDBLOCK) {
                netloader_error("accept", errcode);
                return -1;
            }
#else
            if ( errno != EWOULDBLOCK && errno != EAGAIN) {
                netloader_error("accept", errno);
                return -1;
            }
#endif

        }
        else
        {
            if (set_socket_nonblocking(netloader_datafd) == -1)
            {
                netloader_socket_error("set_socket_nonblocking(netloader_datafd)");
                return -1;
            }

            close(netloader_listenfd);
            netloader_listenfd = -1;
            return 1;
        }
    }

    return 0;
}

void netloaderGetState(netloaderState *state) {
    if(state==NULL)return;
    mtx_lock(&netloader_mtx);

    state->activated = netloader_activated;
    state->launch_app = netloader_launchapp;
    state->me = &netloader_me;

    state->transferring = (netloader_datafd >= 0 && netloader_filelen);
    state->sock_connected = netloader_datafd >= 0;
    state->filelen = netloader_filelen;
    state->filetotal = netloader_filetotal;

    memset(state->errormsg, 0, sizeof(state->errormsg));
    if(netloader_errortext[0]) {
        strncpy(state->errormsg, (char*)netloader_errortext, sizeof(state->errormsg)-1);
        memset((char*)netloader_errortext, 0, sizeof(netloader_errortext));
    }

    mtx_unlock(&netloader_mtx);
}

static bool netloaderGetExit(void) {
    bool flag;
    mtx_lock(&netloader_mtx);
    flag = netloader_exitflag;
    mtx_unlock(&netloader_mtx);
    return flag;
}

void netloaderSignalExit(void) {
    if (!netloader_initialized) return;

    mtx_lock(&netloader_mtx);
    netloader_exitflag = 1;
    mtx_unlock(&netloader_mtx);
}

Result netloaderInit(void) {
    Result rc=0;
    if (netloader_initialized) return 0;

    if (mtx_init(&netloader_mtx, mtx_plain) != thrd_success) return 1;

#ifdef __SWITCH__
    rc = socketInitializeDefault();
#endif

#ifdef __WIN32__
    WSADATA wsa_data;
    if (WSAStartup (MAKEWORD(2,2), &wsa_data)) {
        //netloader_error("WSAStartup failed\n",1);
        rc = 2;
    }
#endif

    if (rc) {
        mtx_destroy(&netloader_mtx);
        return rc;
    }

    netloader_initialized = 1;
    return 0;
}

void netloaderExit(void) {
    if (!netloader_initialized) return;
    netloader_initialized = 0;

    mtx_destroy(&netloader_mtx);

#ifdef __SWITCH__
    socketExit();
#endif

#ifdef __WIN32__
    WSACleanup ();
#endif
}

void netloaderTask(void* arg) {
    int ret=0;
    struct sockaddr_in sa_remote;
    struct timespec duration = {.tv_nsec = 100000000};
    menuEntryInit(&netloader_me,ENTRY_TYPE_FILE);

    mtx_lock(&netloader_mtx);
    netloader_exitflag = 0;
    netloader_activated = 0;
    netloader_launchapp = 0;
    netloader_filelen = 0;
    netloader_filetotal = 0;
    mtx_unlock(&netloader_mtx);

    if(netloader_activate() == 0) {
        mtx_lock(&netloader_mtx);
        netloader_activated = 1;
        mtx_unlock(&netloader_mtx);
    }
    else {
        netloader_deactivate();
        return;
    }

    while((ret = netloader_loop(&sa_remote)) == 0 && !netloaderGetExit()) {
        thrd_sleep(&duration, NULL);
    }

    if(ret == 1 && !netloaderGetExit()) {
        int result = loadnro(&netloader_me, netloader_datafd,sa_remote.sin_addr);
        if (result== 0) {
            ret = 1;
        } else {
            ret = -1;
        }
    }

    netloader_deactivate();
    mtx_lock(&netloader_mtx);
    if (ret==1 && !netloader_exitflag) netloader_launchapp = 1;//Access netloader_exitflag directly since the mutex is already locked.
    netloader_exitflag = 0;
    netloader_activated = 0;
    mtx_unlock(&netloader_mtx);
}

