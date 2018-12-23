#pragma once
#include "debug.hpp"
#include <functional>

enum class LANPacketType : u8 {
    Scan,
    ScanResp,
    Connect,
    SyncNetwork,
};

typedef std::function<int(LANPacketType, const void *, size_t)> ReplyFunc;
typedef std::function<int(LANPacketType, const void *, size_t, ReplyFunc)> MessageCallback;

class Pollable {
    public:
        virtual int getFd() = 0;
        virtual int onRead() = 0;
        virtual void onClose() = 0;
        static int Poll(Pollable *fds[], size_t nfds, int timeout = 100);
};

struct LANPacketHeader {
    u32 magic;
    LANPacketType type;
    u8 compressed;
    u16 length;
    u16 decompress_length;
    u8 _reserved[2];
};
class LanSocket {
    protected:
        static const int BufferSize = 2048;
        static const u32 LANMagic = 0x11451400;
        int fd;
        u8 buffer[BufferSize];
        u16 recvSize;
        void resetRecvSize();
        void prepareHeader(LANPacketHeader &header, LANPacketType type);
        int compress(const void *input, size_t input_size, uint8_t *output, size_t *output_size);
        int decompress(const void *input, size_t input_size, uint8_t *output, size_t *output_size);
        int recvPartPacket(u8 *buffer, size_t bufLen, struct sockaddr_in *addr);
        virtual ssize_t recvfrom(void *buf, size_t len, struct sockaddr_in *addr) = 0;
        virtual int sendto(const void *buf, size_t len, struct sockaddr_in *addr) = 0;
    public:
        LanSocket(int fd) : fd(fd), recvSize(0) {};
        ~LanSocket();
        int sendPacket(LANPacketType type, const void *data, size_t size);
        int sendPacket(LANPacketType type, const void *data, size_t size, struct sockaddr_in *addr);
        int recvPacket(MessageCallback callback);
        void close();
        bool isClosed() { return this->fd == -1; };
        int getFd() { return this->fd; };
};
class TcpLanSocketBase : public LanSocket {
    protected:
        virtual ssize_t recvfrom(void *buf, size_t len, struct sockaddr_in *addr);
        virtual int sendto(const void *buf, size_t len, struct sockaddr_in *addr);
    public:
        TcpLanSocketBase(int fd) : LanSocket(fd) {};
};
class UdpLanSocketBase : public LanSocket {
    protected:
        virtual u32 getBroadcast() = 0;
        virtual ssize_t recvfrom(void *buf, size_t len, struct sockaddr_in *addr);
        virtual int sendto(const void *buf, size_t len, struct sockaddr_in *addr);
        u16 listenPort;
    public:
        int sendBroadcast(LANPacketType type, const void *data, size_t size);
        int sendBroadcast(LANPacketType type);
        UdpLanSocketBase(int fd, u16 listenPort) : LanSocket(fd), listenPort(listenPort) {};
};
