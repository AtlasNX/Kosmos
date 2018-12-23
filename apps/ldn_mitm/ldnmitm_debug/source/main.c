#include <stdio.h>
#include <stdlib.h>
#include <switch.h>
#include <string.h>
#include "ldn.h"

#define MODULEID 0x233

Result scanTest(u16 num) {
    Service ldnSrv;
    UserLocalCommunicationService ldnU;
    Result rc = 0;

    rc = smGetService(&ldnSrv, "ldn:u");
    if (R_FAILED(rc)) {
        goto quit;
    }
    
    printf("ldnCreateUserLocalCommunicationService\n");
    rc = ldnCreateUserLocalCommunicationService(&ldnSrv, &ldnU);
    if (R_FAILED(rc)) {
        goto quit_srv;
    }

    printf("ldnInitialize\n");
    rc = ldnInitialize(&ldnU);
    if (R_FAILED(rc)) {
        goto quit_ldn;
    }

    printf("ldnOpenStation\n");
    rc = ldnOpenStation(&ldnU);
    if (R_FAILED(rc)) {
        goto quit_inf;
    }

    u32 state = 0;
    ldnGetState(&ldnU, &state);
    printf("Initialize succeed state: %d\n", state);

    size_t size = 0x480 * num;
    u8 nothing[0x60];
    u16 out;
    u8 *outBuf = malloc(size);
    memset(nothing, 0, 0x60);
    printf("ldnScan\n");
    rc = ldnScan(&ldnU, 0, nothing, &out, outBuf, size);
    free(outBuf);
    if (R_FAILED(rc)) {
        printf("Failed 1\n");
        goto quit_inf;
    }

    printf("Scan succeed. out: %d\n", out);

quit_inf:
    
quit_ldn:
    serviceClose(&ldnU.s);
quit_srv:
    serviceClose(&ldnSrv);
quit:
    return rc;
}

Result saveLogToFile() {
    char version[32];
    Result rc = 0;
    rc = ldnMitmGetVersion(version);
    if (R_FAILED(rc)) {
        return rc;
    }
    printf("LdnMitm version: %s\n", version);
    rc = ldnMitmSaveLogToFile();
    if (R_FAILED(rc)) {
        printf("Save log to file failed\n");
        return rc;
    }

    return rc;
}

int main() {
    gfxInitDefault();
    consoleInit(NULL);

    printf("ldnmitm_debug\n");
    printf("Press A: test ldn scan command\n");
    printf("Press Y: print version and export log to sd card\n");
    printf("Press B: exit\n");

    u16 num = 1;
    while(appletMainLoop()) {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_B) {
            break;
        }

        if (kDown & KEY_Y) {
            printf("Start saveLogToFile\n");
            Result rc = saveLogToFile();
            printf("saveLogToFile exited with %x\n", rc);
        }

        if (kDown & KEY_A) {
            printf("Start scanTest(%d)\n", num);
            Result rc = scanTest(num);
            printf("scanTest(%d) exited with %x\n", num, rc);
            num++;
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
    }

    gfxExit();
    return 0;
}