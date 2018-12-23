#pragma once

#include "common.h"
#include <libconfig.h>

typedef struct
{
    color_t textColor;
    color_t frontWaveColor;
    color_t middleWaveColor;
    color_t backWaveColor;
    color_t backgroundColor;
    color_t highlightColor;
    color_t separatorColor;
    color_t borderColor;
    color_t borderTextColor;
    color_t progressBarColor;
    bool enableWaveBlending;
    char buttonAText[32];
    char buttonBText[32];
    char buttonYText[32];
    char buttonPText[32];
    char buttonMText[32];
    const uint8_t *hbmenuLogoImage;
} theme_t;

typedef enum
{
    THEME_PRESET_LIGHT,
    THEME_PRESET_DARK,
} ThemePreset;

bool colorFromSetting(config_setting_t *rgba, color_t *col);
void themeStartup(ThemePreset preset);
void GetThemePathFromConfig(char* themePath, size_t size);
void SetThemePathToConfig(const char* themePath);

extern theme_t themeCurrent;

extern ThemePreset themeGlobalPreset;
