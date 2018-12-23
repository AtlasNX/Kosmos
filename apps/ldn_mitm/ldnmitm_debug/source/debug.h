#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <switch.h>
#include <string.h>

#define MIN(a, b) (((a) > (b)) ? (b) : (a))

static void printHex(const void *data, size_t size) {
    u8 tls[0x100];
    char buf[128];
    const u8 *d = data;
memcpy(tls, armGetTls(), 0x100);
    printf("printHex %p 0x%lx\n", data, size);
memcpy(armGetTls(), tls, 0x100);
    for (int i = 0; i < size; i+= 16) {
        buf[0] = 0;
        int s = MIN(size - i, 16);
        for (int j = 0; j < s; j++) {
            sprintf(buf + strlen(buf), "%02x", d[i + j]);
        }
memcpy(tls, armGetTls(), 0x100);
        puts(buf);
memcpy(armGetTls(), tls, 0x100);
    }
}