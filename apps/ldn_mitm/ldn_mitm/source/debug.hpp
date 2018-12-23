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
 
#pragma once
#include <switch.h>
#include <inttypes.h>
#include <stdio.h>

void LogHex(const void *data, int size);
void fatalLater(Result err);
bool GetCurrentTime(u64 *out);
bool SaveLogToFile();

template<typename... TS>
void LogFormat(const char *fmt, TS... args) {
    void LogStr(const char *str);
    char buf[256];
    char mfmt[128];
    u64 ts;
    int rc = 0;

    if (!GetCurrentTime(&ts)) {
        LogStr("failed to get time\n");
        return;
    }

    rc = snprintf(mfmt, sizeof(mfmt), "[%" PRIu64 "] %s\n", ts, fmt);
    if (rc < 0 || rc >= static_cast<int>(sizeof(mfmt))) {
        LogStr("fmt too long\n");
        return;
    }

    rc = snprintf(buf, sizeof(buf), mfmt, args...);
    if (rc < 0 || rc >= static_cast<int>(sizeof(buf))) {
        LogStr("result string too long\n");
        return;
    }

    LogStr(buf);
}
