#include <time.h>
#include "common.h"
#include "netloader.h"

#ifndef __SWITCH__
#include "switch/runtime/nxlink.h"
#endif

char rootPathBase[PATH_MAX];
char rootPath[PATH_MAX+8];
void computeFrontGradient(color_t baseColor, int height);

char *menuGetRootPath(void) {
    return rootPath;
}

char *menuGetRootBasePath(void) {
    return rootPathBase;
}

void launchMenuEntryTask(menuEntry_s* arg) {
    menuEntry_s* me = arg;
    if (me->type == ENTRY_TYPE_FOLDER)
        menuScan(me->path);
        //changeDirTask(me->path);
    else if(me->type == ENTRY_TYPE_THEME)
        launchApplyThemeTask(me);
    else
        launchMenuEntry(me);
}

static enum
{
    HBMENU_DEFAULT,
    HBMENU_NETLOADER_ACTIVE,
    HBMENU_NETLOADER_ERROR,
    HBMENU_NETLOADER_SUCCESS,
    HBMENU_THEME_MENU,
} hbmenu_state = HBMENU_DEFAULT;

void launchMenuNetloaderTask() {
    if(hbmenu_state == HBMENU_DEFAULT)
        workerSchedule(netloaderTask, NULL);
}

void launchMenuBackTask() {
    if(hbmenu_state == HBMENU_NETLOADER_ACTIVE) {
        netloaderSignalExit();
    }
    else if(hbmenu_state == HBMENU_THEME_MENU) {
        hbmenu_state = HBMENU_DEFAULT;
        menuScan(rootPath);
    }
    else {
        menuScan("..");
    }

}

void menuHandleAButton(void) {
    menu_s* menu = menuGetCurrent();

    if (hbmenu_state != HBMENU_NETLOADER_ACTIVE && menuIsMsgBoxOpen()) {
        menuCloseMsgBox();
    }
    else if (menu->nEntries > 0 && (hbmenu_state == HBMENU_DEFAULT || hbmenu_state == HBMENU_THEME_MENU))
    {
        int i;
        menuEntry_s* me;
        for (i = 0, me = menu->firstEntry; i != menu->curEntry; i ++, me = me->next);
        launchMenuEntryTask(me);
        //workerSchedule(launchMenuEntryTask, me);
    }
}

void launchApplyThemeTask(menuEntry_s* arg) {
    const char* themePath = arg->path;
    SetThemePathToConfig(themePath);
    themeStartup(themeGlobalPreset);
    computeFrontGradient(themeCurrent.frontWaveColor, 280); 
}

bool menuIsNetloaderActive(void) {
    return hbmenu_state == HBMENU_NETLOADER_ACTIVE;
}

//Draws an RGB888 or RGBA8888 image.
static void drawImage(int x, int y, int width, int height, const uint8_t *image, ImageMode mode) {
    int tmpx, tmpy;
    int pos;
    color_t current_color;

    for (tmpy=0; tmpy<height; tmpy++) {
        for (tmpx=0; tmpx<width; tmpx++) {
            switch (mode) {
                case IMAGE_MODE_RGB24:
                    pos = ((tmpy*width) + tmpx) * 3;
                    current_color = MakeColor(image[pos+0], image[pos+1], image[pos+2], 255);
                    break;

                case IMAGE_MODE_RGBA32:
                    pos = ((tmpy*width) + tmpx) * 4;
                    current_color = MakeColor(image[pos+0], image[pos+1], image[pos+2], image[pos+3]);
                    break;
            }

            DrawPixel(x+tmpx, y+tmpy, current_color);
        }
    }
}

//Draws an RGBA8888 image masked by the passed color.
static void drawIcon(int x, int y, int width, int height, const uint8_t *image, color_t color) {
    int tmpx, tmpy;
    int pos;
    color_t current_color;

    for (tmpy=0; tmpy<height; tmpy++) {
        for (tmpx=0; tmpx<width; tmpx++) {
            pos = ((tmpy*width) + tmpx) * 4;
            current_color = MakeColor(color.r, color.g, color.b, image[pos+3]);
            DrawPixel(x+tmpx, y+tmpy, current_color);
        }
    }
}

uint8_t *folder_icon_small;
uint8_t *invalid_icon_small;
uint8_t *theme_icon_small;

static void drawEntry(menuEntry_s* me, int off_x, int is_active) {
    int x, y;
    int start_y = 720 - 100 - 145;//*(n % 2);
    int end_y = start_y + 140 + 32;
    int start_x = off_x;//(n / 2);
    int end_x = start_x + 140;
    int j;

    const uint8_t *smallimg = NULL;
    const uint8_t *largeimg = NULL;
    char tmpstr[1024];

    int border_start_x, border_end_x;
    int border_start_y, border_end_y;
    color_t border_color = themeCurrent.borderColor;

    int shadow_start_y, shadow_y;
    int shadow_inset;
    color_t shadow_color;
    uint8_t shadow_alpha_base = 80;
    float highlight_multiplier;
    int shadow_size = 4;

    if (is_active) {
        highlight_multiplier = fmax(0.0, fabs(fmod(menuTimer, 1.0) - 0.5) / 0.5);
        border_color = MakeColor(themeCurrent.highlightColor.r + (255 - themeCurrent.highlightColor.r) * highlight_multiplier, themeCurrent.highlightColor.g + (255 - themeCurrent.highlightColor.g) * highlight_multiplier, themeCurrent.highlightColor.b + (255 - themeCurrent.highlightColor.b) * highlight_multiplier, 255);
        border_start_x = start_x-6;
        border_end_x = end_x+6;
        border_start_y = start_y-5;
        border_end_y = end_y+5;
    }
    else {
        border_start_x = start_x-4;
        border_end_x = end_x+4;
        border_start_y = start_y-3;
        border_end_y = end_y+3;
    }

    //{
        for (x=border_start_x; x<border_end_x; x+=4) {
            Draw4PixelsRaw(x, end_y    , border_color);
            Draw4PixelsRaw(x, start_y-1, border_color);
            Draw4PixelsRaw(x, end_y  +1, border_color);
            Draw4PixelsRaw(x, start_y-2, border_color);
            Draw4PixelsRaw(x, end_y  +2, border_color);
            Draw4PixelsRaw(x, start_y-3, border_color);
            Draw4PixelsRaw(x, end_y  +3, border_color);

            if (is_active) {
                Draw4PixelsRaw(x, start_y-3, border_color);
                Draw4PixelsRaw(x, end_y  +3, border_color);
                Draw4PixelsRaw(x, start_y-4, border_color);
                Draw4PixelsRaw(x, end_y  +4, border_color);
                Draw4PixelsRaw(x, start_y-5, border_color);
                Draw4PixelsRaw(x, end_y  +5, border_color);
                shadow_start_y = 6;
            }
            else {
                shadow_start_y = 4;
            }

            for (shadow_y=shadow_start_y; shadow_y < shadow_start_y+shadow_size; shadow_y++) {
                shadow_color = MakeColor(0, 0, 0, shadow_alpha_base * (1.0 - (float)(shadow_y - shadow_start_y) / ((float)shadow_size)));
                shadow_inset =(shadow_y-shadow_start_y);

                if (x >= border_start_x + shadow_inset && x < border_end_x - shadow_inset) {
                    for (j=0; j<4; j++) DrawPixel(x+j, end_y  +shadow_y, shadow_color);
                }
            }
        }

        for (y=border_start_y; y<border_end_y; y++) {
            DrawPixelRaw(start_x  , y, border_color);
            DrawPixelRaw(end_x    , y, border_color);
            DrawPixelRaw(start_x-1, y, border_color);
            DrawPixelRaw(end_x  +1, y, border_color);
            DrawPixelRaw(start_x-2, y, border_color);
            DrawPixelRaw(end_x  +2, y, border_color);
            DrawPixelRaw(start_x-3, y, border_color);
            DrawPixelRaw(end_x  +3, y, border_color);
            DrawPixelRaw(start_x-4, y, border_color);

            if (is_active) {
                DrawPixelRaw(end_x  +4, y, border_color);
                DrawPixelRaw(start_x-5, y, border_color);
                DrawPixelRaw(end_x  +5, y, border_color);
                DrawPixelRaw(start_x-6, y, border_color);
            }
        }
    //}

    for (y=start_y; y<end_y; y++) {
        for (x=start_x; x<end_x; x+=4) {
            Draw4PixelsRaw(x, y, themeCurrent.borderColor);
        }
    }

    if (me->icon_gfx_small && me->icon_gfx) {
        smallimg = me->icon_gfx_small;
        largeimg = me->icon_gfx;
    }
    else if (me->type == ENTRY_TYPE_FOLDER) {
        smallimg = folder_icon_small;
        largeimg = assetsGetDataBuffer(AssetId_folder_icon);
    }
    else if (me->type == ENTRY_TYPE_THEME){
        smallimg = theme_icon_small;
        if(themeGlobalPreset == THEME_PRESET_DARK)
            largeimg = assetsGetDataBuffer(AssetId_theme_icon_dark);
        else largeimg = assetsGetDataBuffer(AssetId_theme_icon_light);
    }
    else {
        smallimg = invalid_icon_small;
        largeimg = assetsGetDataBuffer(AssetId_invalid_icon);
    }

    if (smallimg) {
        drawImage(start_x, start_y + 32, 140, 140, smallimg, IMAGE_MODE_RGB24);
    }

    if (is_active && largeimg) {
        drawImage(117, 100, 256, 256, largeimg, IMAGE_MODE_RGB24);

        shadow_start_y = 100+256;
        border_start_x = 117;
        border_end_x = 117+256;

        for (shadow_y=shadow_start_y; shadow_y <shadow_start_y+shadow_size; shadow_y++) {
            for (x=border_start_x; x<border_end_x; x++) {
                shadow_color = MakeColor(0, 0, 0, shadow_alpha_base * (1.0 - (float)(shadow_y - shadow_start_y) / ((float)shadow_size)));
                shadow_inset =(shadow_y-shadow_start_y);

                if (x >= border_start_x + shadow_inset && x <= border_end_x - shadow_inset) {
                    DrawPixel(x, shadow_y, shadow_color);
                }
            }
        }
    }

    DrawTextTruncate(interuiregular14, start_x + 4, start_y + 4 + 18, themeCurrent.borderTextColor, me->name, 140 - 32, "...");

    if (is_active) {
        start_x = 1280 - 790;
        start_y = 135;

        DrawTextTruncate(interuimedium30, start_x, start_y + 39, themeCurrent.textColor, me->name, 1280 - start_x - 120 ,"...");

        if (me->type != ENTRY_TYPE_FOLDER) {
            memset(tmpstr, 0, sizeof(tmpstr));
            snprintf(tmpstr, sizeof(tmpstr)-1, "%s: %s", textGetString(StrId_AppInfo_Author), me->author);
            DrawText(interuiregular14, start_x, start_y + 28 + 30 + 18, themeCurrent.textColor, tmpstr);
            memset(tmpstr, 0, sizeof(tmpstr));
            snprintf(tmpstr, sizeof(tmpstr)-1, "%s: %s", textGetString(StrId_AppInfo_Version), me->version);
            DrawText(interuiregular14, start_x, start_y + 28 + 30 + 18 + 6 + 18, themeCurrent.textColor, tmpstr);
        }
    }
}

color_t frontWaveGradient[720];

void computeFrontGradient(color_t baseColor, int height) {
    int y;
    int alpha;
    float dark_mult, dark_sub = 75;
    color_t color;

    for (y=0; y<720; y++) {
        alpha = y - (720 - height);

        if (alpha < 0)
            color = baseColor;
        else {
            dark_mult = clamp((float)(alpha - 50) / (float)height, 0.0, 1.0);
            color = MakeColor(baseColor.r - dark_sub * dark_mult, baseColor.g - dark_sub * dark_mult, baseColor.b - dark_sub * dark_mult, 255);
        }

        frontWaveGradient[y] = color;
    }
}

void menuStartupPath(void) {
    char tmp_path[PATH_MAX+28];

    #ifdef __SWITCH__
    strncpy(rootPathBase, "sdmc:", sizeof(rootPathBase)-1);
    #else
    getcwd(rootPathBase, sizeof(rootPathBase));
    #endif
    snprintf(rootPath, sizeof(rootPath)-1, "%s%s%s", rootPathBase, DIRECTORY_SEPARATOR, "switch");

    struct stat st = {0};

    if (stat(rootPath, &st) == -1) {
        mkdir(rootPath, 0755);
    }

    snprintf(tmp_path, sizeof(tmp_path)-1, "%s/config/nx-hbmenu/themes", rootPathBase);
    if (stat(tmp_path, &st) == -1) {
        snprintf(tmp_path, sizeof(tmp_path)-1, "%s/config", rootPathBase);
        mkdir(tmp_path, 0755);

        snprintf(tmp_path, sizeof(tmp_path)-1, "%s/config/nx-hbmenu", rootPathBase);
        mkdir(tmp_path, 0755);

        snprintf(tmp_path, sizeof(tmp_path)-1, "%s/config/nx-hbmenu/themes", rootPathBase);
        mkdir(tmp_path, 0755);
    }

    snprintf(tmp_path, sizeof(tmp_path)-1, "%s/config/nx-hbmenu/fileassoc", rootPathBase);
    if (stat(tmp_path, &st) == -1) {
        mkdir(tmp_path, 0755);
    }
}

void menuStartup(void) {
    char tmp_path[PATH_MAX+28];

    snprintf(tmp_path, sizeof(tmp_path)-1, "%s/config/nx-hbmenu/fileassoc", rootPathBase);
    menuFileassocScan(tmp_path);

    menuScan(rootPath);

    folder_icon_small = downscaleImg(assetsGetDataBuffer(AssetId_folder_icon), 256, 256, 140, 140, IMAGE_MODE_RGB24);
    invalid_icon_small = downscaleImg(assetsGetDataBuffer(AssetId_invalid_icon), 256, 256, 140, 140, IMAGE_MODE_RGB24);
    if(themeGlobalPreset == THEME_PRESET_DARK)
        theme_icon_small = downscaleImg(assetsGetDataBuffer(AssetId_theme_icon_dark), 256, 256, 140, 140, IMAGE_MODE_RGB24);
    else
        theme_icon_small = downscaleImg(assetsGetDataBuffer(AssetId_theme_icon_light), 256, 256, 140, 140, IMAGE_MODE_RGB24);
    computeFrontGradient(themeCurrent.frontWaveColor, 280);
    //menuCreateMsgBox(780, 300, "This is a test");
}

void themeMenuStartup(void) {
    if(hbmenu_state != HBMENU_DEFAULT) return;
    hbmenu_state = HBMENU_THEME_MENU;
    char tmp_path[PATH_MAX+25];

    snprintf(tmp_path, sizeof(tmp_path)-1, "%s%s%s%s%s%s%s", rootPathBase, DIRECTORY_SEPARATOR, "config", DIRECTORY_SEPARATOR, "nx-hbmenu" , DIRECTORY_SEPARATOR, "themes");

    themeMenuScan(tmp_path);
}

color_t waveBlendAdd(color_t a, color_t b, float alpha) {
    return MakeColor(a.r*(1.0f-alpha) + b.r*alpha, a.g*(1.0f-alpha) + b.g*alpha, a.b*(1.0f-alpha) + b.b*alpha, 255);
}

void drawWave(int id, float timer, color_t color, int height, float phase, float speed) {
    int x, y;
    float wave_top_y, alpha, one_minus_alpha;
    color_t existing_color, new_color;

    height = 720 - height;

    for (x=0; x<1280; x++) {
        wave_top_y = approxSin(x*speed/1280.0+timer+phase) * 10.0 + height;

        for (y=wave_top_y; y<720; y++) {
            if (id != 2 && y > wave_top_y + 30)
                break;

            alpha = y-wave_top_y;

            if (themeCurrent.enableWaveBlending) {
                existing_color = FetchPixelColor(x, y);
                new_color = waveBlendAdd(existing_color, color, clamp(alpha, 0.0, 1.0) * 0.3);
            }
            else if (alpha >= 0.3) {
                if (id == 2) {
                    new_color = frontWaveGradient[y];
                }
                else { // no anti-aliasing
                    new_color = color;
                }
            }
            else { // anti-aliasing
                existing_color = FetchPixelColor(x, y);
                alpha = fabs(alpha);
                one_minus_alpha = (1.0 - alpha);
                new_color = MakeColor(color.r * one_minus_alpha + existing_color.r * alpha, color.g * one_minus_alpha + existing_color.g * alpha, color.b * one_minus_alpha + existing_color.b * alpha, 255);
            }

            DrawPixelRaw(x, y, new_color);
        }
    }
}

void drawTime() {

    char timeString[9];

    time_t unixTime = time(NULL);
    struct tm* timeStruct = localtime((const time_t *)&unixTime);

    int hours = timeStruct->tm_hour;
    int minutes = timeStruct->tm_min;
    int seconds = timeStruct->tm_sec;

    sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);

    int tmpX = GetTextXCoordinate(interuimedium20, 1180, timeString, 'r');

    DrawText(interuimedium20, tmpX, 0 + 47 + 10, themeCurrent.textColor, timeString);

}

void drawCharge() {
    char chargeString[5];
    uint32_t batteryCharge;
    bool isCharging;
    bool validPower;

    validPower = powerGetDetails(&batteryCharge, &isCharging);
    
    if (validPower)
    {
        batteryCharge = (batteryCharge > 100) ? 100 : batteryCharge;

        sprintf(chargeString, "%d%%", batteryCharge);
        
        int tmpX = GetTextXCoordinate(interuiregular14, 1180, chargeString, 'r');

        DrawText(interuiregular14, tmpX - 15, 0 + 47 + 10 + 21, themeCurrent.textColor, chargeString);
        drawIcon(1180 - 11, 0 + 47 + 10 + 6, 10, 15, assetsGetDataBuffer(AssetId_battery_icon), themeCurrent.textColor);
        if (isCharging)
            drawIcon(tmpX - 32, 0 + 47 + 10 + 6, 9, 15, assetsGetDataBuffer(AssetId_charging_icon), themeCurrent.textColor);
    }
}

void drawButtons(menu_s* menu, bool emptyDir, int *x_image_out) {
    int x_image = 1280 - 252 - 30 - 32;
    int x_text = 1280 - 216 - 30 - 32;

    if(emptyDir) {
        x_image = 1280 - 126 - 30 - 32;
         x_text = 1280 - 90 - 30 - 32;
    }

    #ifdef __SWITCH__
    if (strcmp( menu->dirname, "sdmc:/") != 0)
    #else
    if (strcmp( menu->dirname, "/") != 0)
    #endif
    {
        //drawImage(x_image, 720 - 48, 32, 32, themeCurrent.buttonBImage, IMAGE_MODE_RGBA32);
        DrawText(fontscale7, x_image, 720 - 47 + 26, themeCurrent.textColor, themeCurrent.buttonBText);//Display the 'B' button from SharedFont.
        DrawText(interuimedium20, x_text, 720 - 47 + 26, themeCurrent.textColor, textGetString(StrId_Actions_Back));
    }

    if(hbmenu_state == HBMENU_DEFAULT)
    {
        x_text = GetTextXCoordinate(interuiregular18, x_image - 32, textGetString(StrId_NetLoader), 'r');
        x_image = x_text - 36;
        *x_image_out = x_image - 40;

        DrawText(fontscale7, x_image, 720 - 47 + 26, themeCurrent.textColor, themeCurrent.buttonYText);
        DrawText(interuiregular18, x_text, 720 - 47 + 26, themeCurrent.textColor, textGetString(StrId_NetLoader));

        x_text = GetTextXCoordinate(interuiregular18, x_image - 32, textGetString(StrId_ThemeMenu), 'r');
        x_image = x_text - 36;
        *x_image_out = x_image - 40;

        DrawText(fontscale7, x_image, 720 - 47 + 26, themeCurrent.textColor, themeCurrent.buttonMText);
        DrawText(interuiregular18, x_text, 720 - 47 + 26, themeCurrent.textColor, textGetString(StrId_ThemeMenu));
    }
}

void menuUpdateNetloader(netloaderState *netloader_state) {
    bool enable_progress = 0;
    float progress = 0;
    char netloader_displaytext[260];
    char textbody[256];

    memset(netloader_displaytext, 0, sizeof(netloader_displaytext));
    memset(textbody, 0, sizeof(textbody));

    u32 ip = gethostid();

    if (ip == INADDR_LOOPBACK)
        snprintf(textbody, sizeof(textbody)-1, "%s", textGetString(StrId_NetLoaderOffline));
    else {
        if (!netloader_state->sock_connected)
            snprintf(textbody, sizeof(textbody)-1, textGetString(StrId_NetLoaderActive), ip&0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF, NXLINK_SERVER_PORT);
        else {
            enable_progress = 1;
            progress = (float)netloader_state->filetotal / netloader_state->filelen;
            snprintf(textbody, sizeof(textbody)-1, textGetString(StrId_NetLoaderTransferring), netloader_state->filetotal/1024, netloader_state->filelen/1024);
        }
    }

    snprintf(netloader_displaytext, sizeof(netloader_displaytext)-1, "%s\n\n\n%s", textGetString(StrId_NetLoader), textbody);

    menuMsgBoxSetNetloaderState(1, netloader_displaytext, enable_progress, progress);
}

void menuLoop(void) {
    menuEntry_s* me;
    menu_s* menu = NULL;
    int i;
    int x, y;
    int menupath_x_endpos = 918 + 40;
    netloaderState netloader_state;

    for (y=0; y<450; y++) {
        for (x=0; x<1280; x+=4) {// don't draw bottom pixels as they are covered by the waves
            Draw4PixelsRaw(x, y, themeCurrent.backgroundColor);
        }
    }

    drawWave(0, menuTimer, themeCurrent.backWaveColor, 295, 0.0, 3.0);
    drawWave(1, menuTimer, themeCurrent.middleWaveColor, 290, 2.0, 3.5);
    drawWave(2, menuTimer, themeCurrent.frontWaveColor, 280, 4.0, -2.5);
    menuTimer += 0.05;

    drawImage(40, 20, 140, 60, themeCurrent.hbmenuLogoImage, IMAGE_MODE_RGBA32);
    DrawText(interuiregular14, 180, 46 + 18, themeCurrent.textColor, VERSION);

    #ifdef PERF_LOG_DRAW//Seperate from the PERF_LOG define since this might affect perf.
    extern u64 g_tickdiff_vsync;
    extern u64 g_tickdiff_frame;

    char tmpstr[64];

    snprintf(tmpstr, sizeof(tmpstr)-1, "%lu", g_tickdiff_vsync);
    DrawText(interuiregular14, 180 + 256, 46 + 18, themeCurrent.textColor, tmpstr);

    snprintf(tmpstr, sizeof(tmpstr)-1, "%lu", g_tickdiff_frame);
    DrawText(interuiregular14, 180 + 256, 46 + 16 + 18, themeCurrent.textColor, tmpstr);
    #endif

    drawTime();
    drawCharge();

    memset(&netloader_state, 0, sizeof(netloader_state));
    netloaderGetState(&netloader_state);

    if(hbmenu_state == HBMENU_DEFAULT && netloader_state.activated) {
        hbmenu_state = HBMENU_NETLOADER_ACTIVE;

        menuCloseMsgBox();
        menuCreateMsgBox(780,300, "");
    } else if(hbmenu_state == HBMENU_NETLOADER_ACTIVE && !netloader_state.activated && !netloader_state.launch_app) {
        hbmenu_state = HBMENU_DEFAULT;
        menuScan(".");//Reload the menu since netloader may have deleted the NRO if the transfer aborted.

        menuCloseMsgBox();
        menuMsgBoxSetNetloaderState(0, NULL, 0, 0);

        if (netloader_state.errormsg[0]) menuCreateMsgBox(780,300, netloader_state.errormsg);
    }

    if(hbmenu_state == HBMENU_NETLOADER_ACTIVE) {
        menuUpdateNetloader(&netloader_state);
    }

    menu = menuGetCurrent();

    if (menu->nEntries==0 || hbmenu_state == HBMENU_NETLOADER_ACTIVE)
    {
        if (hbmenu_state == HBMENU_NETLOADER_ACTIVE) {
            if (netloader_state.launch_app) {
                hbmenu_state = HBMENU_DEFAULT;
                menuCloseMsgBox();
                menuMsgBoxSetNetloaderState(0, NULL, 0, 0);
                menuCreateMsgBox(240,240,  textGetString(StrId_Loading));
                launchMenuEntryTask(netloader_state.me);
            }
        } else {
            DrawText(interuiregular14, 64, 128 + 18, themeCurrent.textColor, textGetString(StrId_NoAppsFound_Msg));
        }
        drawButtons(menu, true, &menupath_x_endpos);
    }
    else
    {
        static int v = 0;

        if (menu->nEntries > 7) {
            int wanted_x = clamp(-menu->curEntry * (140 + 30), -(menu->nEntries - 7) * (140 + 30), 0);
            menu->xPos += v;
            v += (wanted_x - menu->xPos) / 3;
            v /= 2;
        }
        else {
            menu->xPos = v = 0;
        }

        menuEntry_s *active_entry = NULL;

        // Draw menu entries
        for (me = menu->firstEntry, i = 0; me; me = me->next, i ++) {
            int entry_start_x = 29 + i * (140 + 30);

            int screen_width = 1280;
            if (entry_start_x >= (screen_width - menu->xPos))
                break;

            int is_active = i==menu->curEntry;

            if (is_active)
                active_entry = me;

            drawEntry(me, entry_start_x + menu->xPos, is_active);
        }

        int getX = GetTextXCoordinate(interuiregular18, 1180, textGetString(StrId_ThemeMenu), 'r');

        if(hbmenu_state == HBMENU_THEME_MENU) {
            DrawText(interuiregular18, getX, 30 + 26 + 32 + 10, themeCurrent.textColor, textGetString(StrId_ThemeMenu));
        } else {
            //DrawText(interuiregular18, getX, 30 + 26 + 32 + 10, themeCurrent.textColor, textGetString(StrId_ThemeMenu));
            //DrawText(fontscale7, getX - 40,  30 + 26 + 32 + 10, themeCurrent.textColor, themeCurrent.buttonMText);
        }
        
        if(active_entry != NULL) {
            if (active_entry->type == ENTRY_TYPE_THEME) {
                DrawText(fontscale7, 1280 - 126 - 30 - 32, 720 - 47 + 24, themeCurrent.textColor, themeCurrent.buttonAText);
                DrawText(interuiregular18, 1280 - 90 - 30 - 32, 720 - 47 + 24, themeCurrent.textColor, textGetString(StrId_Actions_Apply));
            }
            else if (active_entry->type != ENTRY_TYPE_FOLDER) {
                DrawText(fontscale7, 1280 - 126 - 30 - 32, 720 - 47 + 24, themeCurrent.textColor, themeCurrent.buttonAText);//Display the 'A' button from SharedFont.
                DrawText(interuiregular18, 1280 - 90 - 30 - 32, 720 - 47 + 24, themeCurrent.textColor, textGetString(StrId_Actions_Launch));
            }
            else {
                DrawText(fontscale7, 1280 - 126 - 30 - 32, 720 - 47 + 24, themeCurrent.textColor, themeCurrent.buttonAText);
                DrawText(interuiregular18, 1280 - 90 - 30 - 32, 720 - 47 + 24, themeCurrent.textColor, textGetString(StrId_Actions_Open));
            }
        }

        drawButtons(menu, false, &menupath_x_endpos);
    }

    DrawTextTruncate(interuiregular18, 40, 720 - 47 + 24, themeCurrent.textColor, menu->dirname, menupath_x_endpos - 40, "...");

    menuDrawMsgBox();
}
