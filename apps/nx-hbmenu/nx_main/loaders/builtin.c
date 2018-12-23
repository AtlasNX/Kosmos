#include <inttypes.h>

#include "../common/common.h"

static char argBuf[ENTRY_ARGBUFSIZE];

static char *init_args(char *dst, size_t dst_maxsize, u32 *in_args, size_t size)
{
    size_t tmplen;
    u32 argi;
    char *in_argdata = (char*)&in_args[1];

    size-= sizeof(u32);

    for (argi=0; argi<in_args[0]; argi++) {
        if (size < 2) break;

        tmplen = strnlen(in_argdata, size-1);

        if (tmplen+3 > dst_maxsize) break;

        if (dst_maxsize < 3) break;

        *dst++ = '"';
        dst_maxsize--;

        strncpy(dst, in_argdata, tmplen);
        in_argdata+= tmplen+1;
        size-= tmplen+1;
        dst+= tmplen;
        dst_maxsize-= tmplen;

        *dst++ = '"';
        dst_maxsize--;

        if (argi+1 < in_args[0]) {
            *dst++ = ' ';
            dst_maxsize--;
        }
    }
    return dst;
}

static bool init(void)
{
    return envHasNextLoad();
}

static void deinit(void)
{

}

static void launchFile(const char* path, argData_s* args)
{
    char msg[256];
    /*if (strncmp(path, "sdmc:/",6) == 0)
        path += 5;*/
    memset(argBuf, 0, sizeof(argBuf));

   uint32_t remote = args->nxlink_host.s_addr;

   if (remote) {
        char nxlinked[17];
        sprintf(nxlinked,"%08" PRIx32 "_NXLINK_",remote);
        launchAddArg(args, nxlinked);
    }

    init_args(argBuf, sizeof(argBuf)-1, args->buf, sizeof(args->buf));

    Result rc = envSetNextLoad(path, argBuf);
    if(R_FAILED(rc)) {
        memset(msg, 0, sizeof(msg));
        snprintf(msg, sizeof(msg)-1, "%s\n0x%x", textGetString(StrId_AppLaunchError), rc);

        menuCreateMsgBox(780, 300, msg);
    }
    else {
        uiExitLoop();
    }
}

const loaderFuncs_s loader_builtin =
{
    .name = "builtin",
    .init = init,
    .deinit = deinit,
    .launchFile = launchFile,
};

