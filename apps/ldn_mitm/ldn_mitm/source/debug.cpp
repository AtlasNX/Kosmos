/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <switch.h>
#include <cstring>
#include <cstdio>
#include <ctime>
#include "debug.hpp"

const size_t TlsBackupSize = 0x100;
// 1MB
const size_t MemoryLogSize = 0x400 * 0x400;
#ifndef ENABLE_LOG
#define ENABLE_LOG 0
#endif
#ifndef ENABLE_MEMLOG
#define ENABLE_MEMLOG 1
#endif

#if ENABLE_LOG
static Mutex g_file_mutex = 0;
#endif
#if ENABLE_MEMLOG
static char MemoryLog[MemoryLogSize] = {0};
static size_t MemoryLogPos = 0;
static Mutex MemoryLogMutex = 0;
#endif
#define BACKUP_TLS() u8 _tls_backup[TlsBackupSize];memcpy(_tls_backup, armGetTls(), TlsBackupSize);
#define RESTORE_TLS() memcpy(armGetTls(), _tls_backup, TlsBackupSize);

#define MIN(a, b) (((a) > (b)) ? (b) : (a))
void LogStr(const char *str);

void LogHex(const void *data, int size) {
    (void)(data);
    (void)(size);
    /* ... */
#if ENABLE_LOG
    u8 *dat = (u8 *)data;
    char buf[128];
    LogFormat("Bin Log: %d (%p)", size, data);
    for (int i = 0; i < size; i += 16) {
        int s = MIN(size - i, 16);
        buf[0] = 0;
        for (int j = 0; j < s; j++) {
            sprintf(buf + strlen(buf), "%02x", dat[i + j]);
        }
        sprintf(buf + strlen(buf), "\n");
        LogStr(buf);
    }
#endif
}

void LogStr(const char *str) {
    (void)(str);
    BACKUP_TLS();
    size_t len = strlen(str);
#if ENABLE_LOG
    mutexLock(&g_file_mutex);
    FILE *file = fopen("sdmc:/ldn_mitm.log", "ab+");
    if (file) {
        fwrite(str, 1, len, file);
        fclose(file);
    }
    mutexUnlock(&g_file_mutex);
#endif
#if ENABLE_MEMLOG
    mutexLock(&MemoryLogMutex);
    if (MemoryLogPos + len >= MemoryLogSize) {
        size_t dis = 0;
        for (size_t i = 0; i < MemoryLogSize && MemoryLog[i] != '\0'; i++) {
            if (MemoryLog[i] == '\n') {
                dis = i + 1;
                if (MemoryLogPos - dis + len < MemoryLogSize) break;
            }
        }
        if (dis) {
            std::memmove(MemoryLog, MemoryLog + dis, MemoryLogSize - dis);
            MemoryLogPos -= dis;
        } else {
            MemoryLogPos = 0;
        }
        MemoryLog[MemoryLogPos] = 0;
    }
    if (MemoryLogPos + len >= MemoryLogSize) {
        MemoryLogPos = 0;
    }
    std::strcpy(MemoryLog + MemoryLogPos, str);
    MemoryLogPos += len;
    mutexUnlock(&MemoryLogMutex);
#endif
    RESTORE_TLS();
}

bool SaveLogToFile() {
#if ENABLE_MEMLOG
    u64 curtime;
    if (!GetCurrentTime(&curtime)) {
        return false;
    }
    mutexLock(&MemoryLogMutex);
    FILE *file = fopen("sdmc:/ldn_mitm_memlog.log", "ab+");
    if (file) {
        fprintf(file, "ldn_mitm memory log dump\nversion: " GITDESCVER "\ntimestamp: %" PRIu64 "\n\n", curtime);
        fwrite(MemoryLog, 1, MemoryLogPos, file);
        fclose(file);
    }
    MemoryLogPos = 0;
    mutexUnlock(&MemoryLogMutex);
    return true;
#else
    return false;
#endif
}

struct fatalLaterIpc
{
    u64 magic;
    u64 cmd_id;
    u64 result;
    u64 unknown;
};
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

    struct fatalLaterIpc* raw;

    raw = (struct fatalLaterIpc*) ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;
    raw->result = err;
    raw->unknown = 0;

    ipcDispatch(srv);
    svcCloseHandle(srv);
}
bool GetCurrentTime(u64 *out) {
    *out = 0;
    *out = (armGetSystemTick() * 625 / 12) / 1000000;
    return true;
}
