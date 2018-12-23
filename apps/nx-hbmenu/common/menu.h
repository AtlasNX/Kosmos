#pragma once

#ifndef _WIN32
#include <arpa/inet.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#undef DrawText
#undef MessageBox
#endif

#define ENTRY_NAMELENGTH   0x200
#define ENTRY_AUTHORLENGTH 0x100
#define ENTRY_VERLENGTH   0x10
#define ENTRY_ARGBUFSIZE 0x400

typedef enum
{
    ENTRY_TYPE_FILE,
    ENTRY_TYPE_FOLDER,
    ENTRY_TYPE_THEME,
    ENTRY_TYPE_FILEASSOC,
    ENTRY_TYPE_FILE_OTHER,
} MenuEntryType;

typedef struct menuEntry_s_tag menuEntry_s;
typedef struct menu_s_tag menu_s;

struct menu_s_tag
{
    menuEntry_s *firstEntry, *lastEntry;
    int nEntries;
    int curEntry;
    int xPos;

    char dirname[PATH_MAX+1];
};

typedef struct
{
    char* dst;
    uint32_t buf[ENTRY_ARGBUFSIZE/sizeof(uint32_t)];
    struct in_addr nxlink_host;
} argData_s;

struct menuEntry_s_tag
{
    menu_s* menu;
    menuEntry_s* next;
    MenuEntryType type;

    char path[PATH_MAX+8];
    argData_s args;

    bool fileassoc_type;//0=file_extension, 1 = filename
    char fileassoc_str[PATH_MAX+1];//file_extension/filename

    char name[ENTRY_NAMELENGTH+1];
    char author[ENTRY_AUTHORLENGTH+1];
    char version[ENTRY_VERLENGTH+1];

    uint8_t *icon;
    size_t icon_size;
    uint8_t *icon_gfx;
    uint8_t *icon_gfx_small;

    NacpStruct *nacp;
};

typedef enum
{
    IMAGE_MODE_RGB24,
    IMAGE_MODE_RGBA32
} ImageMode;

double menuTimer;

#ifdef __cplusplus
extern "C" {
#endif

void menuEntryInit(menuEntry_s* me, MenuEntryType type);
void menuEntryFree(menuEntry_s* me, bool skip_icongfx);
bool fileExists(const char* path);
bool menuEntryLoad(menuEntry_s* me, const char* name, bool shortcut);
void menuEntryParseIcon(menuEntry_s* me);
uint8_t *downscaleImg(const uint8_t *image, int srcWidth, int srcHeight, int destWidth, int destHeight, ImageMode mode);
void menuEntryParseNacp(menuEntry_s* me);

void menuEntryFileassocLoad(const char* filepath);

menuEntry_s* menuCreateEntry(MenuEntryType type);

void menuFileassocAddEntry(menuEntry_s* me);
void menuDeleteEntry(menuEntry_s* me, bool skip_icongfx);

menu_s* menuGetCurrent(void);
menu_s* menuFileassocGetCurrent(void);
int menuScan(const char* target);
int themeMenuScan(const char* target);
int menuFileassocScan(const char* target);

void launchMenuEntryTask(menuEntry_s* arg);
void launchApplyThemeTask(menuEntry_s* arg);
void launchMenuBackTask();
void launchMenuNetloaderTask();
char *menuGetRootPath(void);
char *menuGetRootBasePath(void);

void menuHandleAButton(void);

bool menuIsNetloaderActive(void);

#ifdef __cplusplus
}
#endif

static inline char* getExtension(const char* str)
{
    const char* p;
    for (p = str+strlen(str); p >= str && *p != '.'; p--);
    return (char*)p;
}

static inline char* getSlash(const char* str)
{
    const char* p;
    for (p = str+strlen(str); p >= str && *p != '/'; p--);
    return (char*)p;
}

