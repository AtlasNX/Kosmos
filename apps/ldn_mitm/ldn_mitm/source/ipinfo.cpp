#include "ipinfo.hpp"
#include <arpa/inet.h>

static Service g_nifmSrv;
static Service g_nifmIGS;
static u64 g_nifmRefCount = 0;
static const int ModuleID = 0x233;

Result ipinfoInit() {
    atomicIncrement64(&g_nifmRefCount);
    if (serviceIsActive(&g_nifmSrv))
        return 0;

    Result rc = smGetService(&g_nifmSrv, "nifm:u");
    if (R_FAILED(rc)) {
        rc = MAKERESULT(ModuleID, 5);
        goto quit;
    }

    IpcCommand c;
    IpcParsedCommand r;

    ipcInitialize(&c);
    ipcSendPid(&c);
    struct {
        u64 magic;
        u64 cmd_id;
        u64 param;
    } *raw;

    raw = (decltype(raw))serviceIpcPrepareHeader(&g_nifmSrv, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 5;
    raw->param = 0;

    rc = serviceIpcDispatch(&g_nifmSrv);

    if (R_FAILED(rc)) {
        rc = MAKERESULT(ModuleID, 6);
        goto quit;
    }
    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_nifmSrv, &r, sizeof(*resp));
    resp = (decltype(resp))r.Raw;

    rc = resp->result;

    if (R_FAILED(rc)) {
        rc = MAKERESULT(ModuleID, 7);
        goto quit;
    }

    serviceCreateSubservice(&g_nifmIGS, &g_nifmSrv, &r, 0);

quit:
    return rc;
}
void ipinfoExit() {
    if (atomicDecrement64(&g_nifmRefCount) == 0) {
        serviceClose(&g_nifmIGS);
        serviceClose(&g_nifmSrv);
    }
}

Result ipinfoGetIpConfig(u32* address) {
    u32 netmask;
    return ipinfoGetIpConfig(address, &netmask);
}

Result ipinfoGetIpConfig(u32* address, u32* netmask) {
    Result rc;
    IpcCommand c;
    IpcParsedCommand r;

    ipcInitialize(&c);
    struct {
        u64 magic;
        u64 cmd_id;
    } *raw;

    raw = (decltype(raw))serviceIpcPrepareHeader(&g_nifmIGS, &c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 15; // GetCurrentIpConfigInfo

    rc = serviceIpcDispatch(&g_nifmIGS);
    if (R_FAILED(rc)) {
        return rc;
    }
    struct {
        u64 magic;
        u64 result;
        u8 _unk;
        u32 address;
        u32 netmask;
        u32 gateway;
    } __attribute__((packed)) *resp;

    serviceIpcParse(&g_nifmIGS, &r, sizeof(*resp));
    resp = (decltype(resp))r.Raw;

    rc = resp->result;
    if (R_FAILED(rc)) {
        return rc;
    }
    *address = ntohl(resp->address);
    *netmask = ntohl(resp->netmask);
    // ret = resp->address | ~resp->netmask;

    return rc;
}
