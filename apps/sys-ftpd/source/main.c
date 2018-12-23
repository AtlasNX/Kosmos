#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>
#include "console.h"
#include "ftp.h"

// only for mkdir, used when creating the "logs" directory
#include <sys/stat.h>

#include <switch.h>

#include "util.h"

#include "mp3.h"

#define TITLE_ID 0x420000000000000E
#define HEAP_SIZE 0x000540000

// we aren't an applet
u32 __nx_applet_type = AppletType_None;

// setup a fake heap
char fake_heap[HEAP_SIZE];

// we override libnx internals to do a minimal init
void __libnx_initheap(void)
{
    extern char *fake_heap_start;
    extern char *fake_heap_end;

    // setup newlib fake heap
    fake_heap_start = fake_heap;
    fake_heap_end = fake_heap + HEAP_SIZE;
}

void registerFspLr()
{
    if (kernelAbove400())
        return;

    Result rc = fsprInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);

    u64 pid;
    svcGetProcessId(&pid, CUR_PROCESS_HANDLE);

    rc = fsprRegisterProgram(pid, TITLE_ID, FsStorageId_NandSystem, NULL, 0, NULL, 0);
    if (R_FAILED(rc))
        fatalLater(rc);
    fsprExit();
}

void __appInit(void)
{
    Result rc;
    svcSleepThread(10000000000L);
    rc = smInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    registerFspLr();
    rc = fsdevMountSdmc();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = timeInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = hidInitialize();
    if (R_FAILED(rc))
        fatalLater(rc);
}

void __appExit(void)
{
    fsdevUnmountAll();
    fsExit();
    smExit();
    audoutExit();
    timeExit();
}

static loop_status_t loop(loop_status_t (*callback)(void))
{
    loop_status_t status = LOOP_CONTINUE;

    while (appletMainLoop())
    {
        //svcSleepThread(30000000L);
        status = callback();
        console_render();
        if (status != LOOP_CONTINUE)
            return status;
        if (isPaused())
            return LOOP_RESTART;
    }
    return LOOP_EXIT;
}

void inputPoller()
{
    while (appletMainLoop())
    {
        svcSleepThread(1e+8L);
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

        if ((kDown & KEY_PLUS || kDown & KEY_MINUS || kDown & KEY_X) && (kHeld & KEY_PLUS && kHeld & KEY_MINUS && kHeld & KEY_X))
            setPaused(!isPaused());
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    FILE *should_log_file = fopen("/logs/ftpd_log_enabled", "r");
    if (should_log_file != NULL)
    {
        should_log = true;
        fclose(should_log_file);

        mkdir("/logs", 0700);
        unlink("/logs/ftpd.log");
    }

    mp3MutInit();
    pauseInit();
    Thread pauseThread;
    Result rc = threadCreate(&pauseThread, inputPoller, NULL, 0x4000, 49, 3);
    if (R_FAILED(rc))
        fatalLater(rc);
    rc = threadStart(&pauseThread);
    if (R_FAILED(rc))
        fatalLater(rc);


    loop_status_t status = LOOP_RESTART;


    ftp_pre_init();
    while (status == LOOP_RESTART)
    {
        while (isPaused())
        {
            svcSleepThread(1000000000L);
        }

        /* initialize ftp subsystem */
        if (ftp_init() == 0)
        {
            /* ftp loop */
            status = loop(ftp_loop);

            /* done with ftp */
            ftp_exit();
        }
        else
            status = LOOP_EXIT;
    }
    ftp_post_exit();

    return 0;
}
