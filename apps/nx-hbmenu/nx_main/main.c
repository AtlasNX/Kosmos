#include <switch.h>
#include <string.h>
#include <stdio.h>

#include "../common/common.h"
#include "nx_touch.h"

uint8_t* g_framebuf;
u32 g_framebuf_width;

bool menuUpdateErrorScreen(void);

#ifdef PERF_LOG
u64 g_tickdiff_vsync=0;
u64 g_tickdiff_frame=0;
#endif

#ifdef ENABLE_AUDIO
void audio_initialize(void);
void audio_exit(void);
#endif

extern u32 __nx_applet_exit_mode;

int main(int argc, char **argv)
{
    bool error_screen=0;
    Result lastret=0;
    Result rc=0;
    char msg[256];
    char errormsg[256];//Can't use StrId for these error messages since it would be unavailable if textInit fails.

    #ifdef PERF_LOG
    u64 start_tick=0;
    #endif

    memset(errormsg, 0, sizeof(errormsg));

    appletLockExit();
    appletSetScreenShotPermission(1);

    ColorSetId theme;
    rc = setsysInitialize();
    if (R_FAILED(rc)) snprintf(errormsg, sizeof(errormsg)-1, "Error: setsysInitialize() failed: 0x%x.", rc);

    if (R_SUCCEEDED(rc)) setsysGetColorSetId(&theme);

    if (R_SUCCEEDED(rc)) {
        rc = plInitialize();
        if (R_FAILED(rc)) snprintf(errormsg, sizeof(errormsg)-1, "Error: plInitialize() failed: 0x%x.", rc);
    }

    if (R_SUCCEEDED(rc)) {
        rc = textInit();
        if (R_FAILED(rc)) {
            snprintf(errormsg, sizeof(errormsg)-1, "Error: textInit() failed: 0x%x.", rc);
        }
    }

    if (R_SUCCEEDED(rc)) menuStartupPath();

    if (R_SUCCEEDED(rc)) {
        rc = assetsInit();
        if (R_FAILED(rc)) {
            snprintf(errormsg, sizeof(errormsg)-1, "Error: assetsInit() failed: 0x%x.", rc);
        }
    }

    if (R_SUCCEEDED(rc)) themeStartup((ThemePreset)theme);

    if (R_SUCCEEDED(rc)) powerInit();

    if (R_SUCCEEDED(rc)) {
        rc = netloaderInit();
        if (R_FAILED(rc)) {
            snprintf(errormsg, sizeof(errormsg)-1, "Error: netloaderInit() failed: 0x%x.", rc);
        }
    }

    if (R_SUCCEEDED(rc) && !workerInit()) {
        rc = 1;
        snprintf(errormsg, sizeof(errormsg)-1, "Error: workerInit() failed.");
    }

    if (R_SUCCEEDED(rc)) menuStartup();

    if (R_SUCCEEDED(rc)) {
        if (!launchInit()) {
            rc = 2;
            snprintf(errormsg, sizeof(errormsg)-1, "Error: launchInit() failed.");
        }
    }

    if (R_SUCCEEDED(rc) && !fontInitialize()) {
        rc = 3;
        snprintf(errormsg, sizeof(errormsg)-1, "Error: fontInitialize() failed.");
    }

    #ifdef ENABLE_AUDIO
    if (R_SUCCEEDED(rc)) audio_initialize();
    #endif

    if (R_SUCCEEDED(rc)) {
        lastret = envGetLastLoadResult();

        if (R_FAILED(lastret)) {
            memset(msg, 0, sizeof(msg));
            snprintf(msg, sizeof(msg)-1, "%s\n0x%x", textGetString(StrId_LastLoadResult), lastret);

            menuCreateMsgBox(780, 300, msg);
        }
    }

    if (errormsg[0]) error_screen = 1;

    if (!error_screen) {
        gfxInitDefault();
    }
    else {
        consoleInit(NULL);
        printf("%s\n", errormsg);
        printf("Press the + button to exit.\n");
    }

    #ifdef PERF_LOG
        if (!error_screen) {
        gfxWaitForVsync();

        start_tick = svcGetSystemTick();
        gfxWaitForVsync();
        g_tickdiff_vsync = svcGetSystemTick() - start_tick;
    }
    #endif

    while (appletMainLoop())
    {
        #ifdef PERF_LOG
        if (!error_screen) start_tick = svcGetSystemTick();
        #endif

        //Scan all the inputs. This should be done once for each frame
        hidScanInput();

        if (!error_screen) {
            g_framebuf = gfxGetFramebuffer(&g_framebuf_width, NULL);
            memset(g_framebuf, 237, gfxGetFramebufferSize());
            if (!uiUpdate()) break;
            menuLoop();
        }
        else {
            if (menuUpdateErrorScreen()) break;
        }

        if (!error_screen) {
            gfxFlushBuffers();

            #ifdef PERF_LOG
            g_tickdiff_frame = svcGetSystemTick() - start_tick;
            #endif

            gfxSwapBuffers();
        }
        else {
            consoleUpdate(NULL);
        }
    }

    if (!error_screen) {
        gfxExit();
    }
    else {
        consoleExit(NULL);
        __nx_applet_exit_mode = 1;
    }

    #ifdef ENABLE_AUDIO
    audio_exit();
    #endif

    fontExit();
    launchExit();
    netloaderSignalExit();
    workerExit();
    netloaderExit();
    powerExit();
    assetsExit();
    plExit();
    setsysExit();

    appletUnlockExit();

    return 0;
}

//This is implemented here due to the hid code.
bool menuUpdate(void) {
    bool exitflag = 0;
    menu_s* menu = menuGetCurrent();
    u32 down = hidKeysDown(CONTROLLER_P1_AUTO);
    handleTouch(menu);

    if (down & KEY_Y)
    {
        launchMenuNetloaderTask();
    }
    else if (down & KEY_A)
    {
        menuHandleAButton();
    }
    else if (down & KEY_B)
    {
        launchMenuBackTask();
    }
    else if(down & KEY_MINUS){
        themeMenuStartup();
    }
    else if (down & KEY_PLUS)
    {
        exitflag = 1;
    }
    else if (menu->nEntries > 0)
    {
        int move = 0;

        if (down & KEY_LEFT) move--;
        if (down & KEY_RIGHT) move++;
        if (down & KEY_DOWN) move-=7;
        if (down & KEY_UP) move+=7;

        int newEntry = menu->curEntry + move;
        if (newEntry < 0) newEntry = 0;
        if (newEntry >= menu->nEntries) newEntry = menu->nEntries-1;
        menu->curEntry = newEntry;
    }

    return exitflag;
}

bool menuUpdateErrorScreen(void) {
    bool exitflag = 0;
    u32 down = hidKeysDown(CONTROLLER_P1_AUTO);

    if (down & KEY_PLUS)
    {
        exitflag = 1;
    }

    return exitflag;
}
