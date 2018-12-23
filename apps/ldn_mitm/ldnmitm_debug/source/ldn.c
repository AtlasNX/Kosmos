#include "ldn.h"
#include "debug.h"
#include <string.h>

void NetworkInfo2NetworkConfig(NetworkInfo* info, NetworkConfig* out) {
    out->intentId = info->networkId.intentId;
    out->channel = info->common.channel;
    out->nodeCountMax = info->ldn.nodeCountMax;
    out->localCommunicationVersion = 1;
}

void NetworkInfo2SecurityParameter(NetworkInfo* info, SecurityParameter* out) {
    out->sessionId = info->networkId.sessionId;
    memcpy(out->unkRandom, info->ldn.unkRandom, 16);
}

Result ldnOpenStation(UserLocalCommunicationService* s) {
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;
    raw = serviceIpcPrepareHeader(&s->s, &c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 300;

    Result rc = serviceIpcDispatch(&s->s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;
        serviceIpcParse(&s->s, &r, sizeof(*resp));
        resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result ldnGetState(UserLocalCommunicationService* s, u32* state) {
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;
    raw = serviceIpcPrepareHeader(&s->s, &c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 0;

    Result rc = serviceIpcDispatch(&s->s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u32 state;
        } *resp;
        serviceIpcParse(&s->s, &r, sizeof(*resp));
        resp = r.Raw;

        rc = resp->result;
        *state = resp->state;
    }

    return rc;
}

Result ldnInitialize(UserLocalCommunicationService* s) {
    IpcCommand c;
    ipcInitialize(&c);

    ipcSendPid(&c);
    struct {
        u64 magic;
        u64 cmd_id;
        u64 unk;
    } *raw;
    raw = serviceIpcPrepareHeader(&s->s, &c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 400;
    raw->unk = 0;

    Result rc = serviceIpcDispatch(&s->s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;
        serviceIpcParse(&s->s, &r, sizeof(*resp));
        resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result ldnScan(UserLocalCommunicationService* s, u16 channel, void* unk2, u16* unkOut, void* outBuf, size_t outLen) {
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
        u16 channel;
        u8 unk2[0x60];
    } *raw;
    ipcAddRecvBuffer(&c, outBuf, outLen, BufferType_Normal);
    ipcAddRecvStatic(&c, 0, 0, BufferType_Normal);
    raw = serviceIpcPrepareHeader(&s->s, &c, sizeof(*raw));
    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 102;
    raw->channel = channel;
    memcpy(raw->unk2, unk2, 0x60);

    printHex(armGetTls(), 0x100);
    Result rc = serviceIpcDispatch(&s->s);
    printHex(armGetTls(), 0x100);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            u16 unk;
        } *resp;
        serviceIpcParse(&s->s, &r, sizeof(*resp));
        resp = r.Raw;
        printHex(resp, sizeof(*resp));

        rc = resp->result;
        *unkOut = resp->unk;
    }

    return rc;
}

Result ldnGetNetworkInfo(UserLocalCommunicationService* s, void* out) {
    IpcCommand c;
    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;
    ipcAddRecvStatic(&c, out, 0x480, 0);
    raw = serviceIpcPrepareHeader(&s->s, &c, sizeof(*raw));
    // LogHex(armGetTls(), 0x100);

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;
    Result rc = serviceIpcDispatch(&s->s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;
        serviceIpcParse(&s->s, &r, sizeof(*resp));
        resp = r.Raw;

        rc = resp->result;
    }

    return rc;
}

Result ldnCreateUserLocalCommunicationService(Service* s, UserLocalCommunicationService* out) {
    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = serviceIpcPrepareHeader(s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 0;

    Result rc = serviceIpcDispatch(s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;
        
        serviceIpcParse(s, &r, sizeof(*resp));
        resp = r.Raw;

        rc = resp->result;

        if (R_SUCCEEDED(rc)) {
            serviceCreateSubservice(&out->s, s, &r, 0);
        }
    }

    return rc;
}

Result ldnMitmGetVersion(char *version) {
    Result rc = 0;
    Service s;

    rc = smGetService(&s, "ldnmitm");
    if (R_FAILED(rc)) {
        goto quit;
    }

    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = serviceIpcPrepareHeader(&s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 65001;

    rc = serviceIpcDispatch(&s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
            char version[32];
        } *resp;
        
        serviceIpcParse(&s, &r, sizeof(*resp));
        resp = r.Raw;

        rc = resp->result;
        if (R_SUCCEEDED(rc)) {
            strcpy(version, resp->version);
        }
    }
    serviceClose(&s);

quit:
    return rc;
}

Result ldnMitmSaveLogToFile() {
    Result rc = 0;
    Service s;

    rc = smGetService(&s, "ldnmitm");
    if (R_FAILED(rc)) {
        goto quit;
    }

    IpcCommand c;
    ipcInitialize(&c);

    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = serviceIpcPrepareHeader(&s, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 65000;

    rc = serviceIpcDispatch(&s);

    if (R_SUCCEEDED(rc)) {
        IpcParsedCommand r;
        struct {
            u64 magic;
            u64 result;
        } *resp;
        
        serviceIpcParse(&s, &r, sizeof(*resp));
        resp = r.Raw;

        rc = resp->result;
    }
    serviceClose(&s);

quit:
    return rc;
}