#pragma once
#include "common.h"

typedef enum {
    AssetId_battery_icon,
    AssetId_charging_icon,
    AssetId_folder_icon,
    AssetId_invalid_icon,
    AssetId_hbmenu_logo_dark,
    AssetId_hbmenu_logo_light,
    AssetId_theme_icon_dark,
    AssetId_theme_icon_light,

    AssetId_Max
} AssetId;

Result assetsInit(void);
void assetsExit(void);
void assetsGetData(AssetId id, u8 **buffer, size_t *size);
u8 *assetsGetDataBuffer(AssetId id);

