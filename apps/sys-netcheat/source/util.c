#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <switch.h>
#include "util.h"

int setupServerSocket()
{
    int lissock;
    struct sockaddr_in server;
    lissock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5555);

    while (bind(lissock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        svcSleepThread(1e+9L);
    }
    listen(lissock, 3);
    return lissock;
}

void fatalLater(Result err)
{
    Handle srv;

    while (R_FAILED(smGetServiceOriginal(&srv, smEncodeName("fatal:u"))))
    {
        // wait one sec and retry
        svcSleepThread(1000000000L);
    }

    // fatal is here time, fatal like a boss
    IpcCommand c;
    ipcInitialize(&c);
    ipcSendPid(&c);
    struct
    {
        u64 magic;
        u64 cmd_id;
        u64 result;
        u64 unknown;
    } * raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;
    raw->result = err;
    raw->unknown = 0;

    ipcDispatch(srv);
    svcCloseHandle(srv);
}
