#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>

#include <switch.h>

#include "mp3.h"

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


bool paused = false;
static Mutex pausedMutex;
void pauseInit() {
    mutexInit(&pausedMutex);
    mutexLock(&pausedMutex);
    FILE *should_pause_file = fopen("/ftpd_paused", "r");
    if (should_pause_file != NULL) {
        paused = true;
        fclose(should_pause_file);
    }
    mutexUnlock(&pausedMutex);
}


bool isPaused() {
    mutexLock(&pausedMutex);
    bool ret = paused;
    mutexUnlock(&pausedMutex);
    return ret;
}

void setPaused(bool newPaused) {
    mutexLock(&pausedMutex);
    paused = newPaused;
    if(paused) {
        FILE *should_pause_file = fopen("/ftpd_paused", "w");
        fclose(should_pause_file);
        playMp3("/ftpd/pauseon.mp3");
    } else {
        unlink("/ftpd_paused");
        playMp3("/ftpd/pauseoff.mp3");
    }
    mutexUnlock(&pausedMutex);
}
