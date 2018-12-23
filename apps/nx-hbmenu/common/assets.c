#include "common.h"

#include <minizip/unzip.h>

typedef struct {
    u8 *buffer;
    size_t size;
    const char *filename;
} assetsDataEntry;

#define GENASSET(x) {.filename = x}

static bool g_assetsInitialized = 0;
assetsDataEntry g_assetsDataList[AssetId_Max] = {
    GENASSET("battery_icon.bin"),
    GENASSET("charging_icon.bin"),
    GENASSET("folder_icon.bin"),
    GENASSET("invalid_icon.bin"),
    GENASSET("hbmenu_logo_dark.bin"),
    GENASSET("hbmenu_logo_light.bin"),
    GENASSET("theme_icon_dark.bin"),
    GENASSET("theme_icon_light.bin"),
};

static void assetsClearEntry(assetsDataEntry *entry) {
    free(entry->buffer);

    entry->size = 0;
    entry->buffer = NULL;
}

static int assetsLoadFile(unzFile zipf, assetsDataEntry *entry) {
    int ret;
    int filesize=0;
    unz_file_info file_info;
    u8* buffer = NULL;

    ret = unzLocateFile(zipf, entry->filename, 0);

    if (ret==UNZ_OK) ret = unzOpenCurrentFile(zipf);

    if (ret==UNZ_OK) {
        ret = unzGetCurrentFileInfo(zipf, &file_info, NULL, 0, NULL, 0, NULL, 0);

        filesize = file_info.uncompressed_size;
        if (filesize == 0) ret = -10;

        if (ret==UNZ_OK) {
            buffer = (u8*)malloc(filesize);
            if (buffer) {
                memset(buffer, 0, filesize);
            } else {
                ret = -11;
            }
        }

        if (ret==UNZ_OK) {
            ret = unzReadCurrentFile(zipf, buffer, filesize);
            if(ret < filesize) {
                ret = -12;
            } else {
                ret = UNZ_OK;
            }
        }

        if (ret!=UNZ_OK && buffer!=NULL) free(buffer);

        unzCloseCurrentFile(zipf);
    }

    if (ret==UNZ_OK) {
        entry->buffer = buffer;
        entry->size = filesize;
    }

    return ret;
}

Result assetsInit(void) {
    int ret=0;
    int i, stopi;
    unzFile zipf;
    assetsDataEntry *entry = NULL;
    char tmp_path[PATH_MAX+1];

    if (g_assetsInitialized) return 0;

    #ifdef __SWITCH__
    Result rc = romfsInit();
    if (R_FAILED(rc)) return rc;
    #endif

    memset(tmp_path, 0, sizeof(tmp_path));

    #ifdef __SWITCH__
    strncpy(tmp_path, "romfs:/assets.zip", sizeof(tmp_path)-1);
    #else
    snprintf(tmp_path, sizeof(tmp_path)-1, "%s/romfs/assets.zip", menuGetRootBasePath());
    #endif

    zipf = unzOpen(tmp_path);
    if(zipf==NULL) {
        #ifdef __SWITCH__
        romfsExit();
        #endif

        return 0x80;
    }

    for (i=0; i<AssetId_Max; i++) {
        stopi = i;
        entry = &g_assetsDataList[i];
        ret = assetsLoadFile(zipf, entry);
        if (ret!=UNZ_OK) break;
    }

    if (ret!=UNZ_OK) {
        for (i=0; i<stopi; i++) {
            assetsClearEntry(&g_assetsDataList[i]);
        }
    }

    if (ret==UNZ_OK) g_assetsInitialized = 1;

    unzClose(zipf);

    #ifdef __SWITCH__
    romfsExit();
    #endif

    return ret;
}

void assetsExit(void) {
    int i;

    if (!g_assetsInitialized) return;
    g_assetsInitialized = 0;

    for (i=0; i<AssetId_Max; i++) {
        assetsClearEntry(&g_assetsDataList[i]);
    }
}

void assetsGetData(AssetId id, u8 **buffer, size_t *size) {
    if (buffer) *buffer = NULL;
    if (size) *size = 0;
    if (id < 0 || id >= AssetId_Max) return;

    assetsDataEntry *entry = &g_assetsDataList[id];

    if (buffer) *buffer = entry->buffer;
    if (size) *size = entry->size;
}

u8 *assetsGetDataBuffer(AssetId id) {
    u8 *buffer = NULL;

    assetsGetData(id, &buffer, NULL);
    return buffer;
}

