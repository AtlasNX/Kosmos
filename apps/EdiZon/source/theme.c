#include "theme.h"

#pragma GCC diagnostic ignored "-Wmissing-braces"

theme_t currTheme;

void setTheme(ColorSetId colorSetId) {
  switch (colorSetId) {
    case ColorSetId_Light:
      currTheme = (theme_t) {
        .textColor            =      RGBA8(0x00, 0x00, 0x00, 0xFF),
        .backgroundColor      =      RGBA8(0xEA, 0xEA, 0xEA, 0xFF),
        .highlightColor       =      RGBA8(0x27, 0xA3, 0xC7, 0xFF),
        .selectedColor        =      RGBA8(0x50, 0x2D, 0xE4, 0xFF),
        .separatorColor       =      RGBA8(0x60, 0x60, 0x60, 0x80),
        .selectedButtonColor  =      RGBA8(0xFD, 0xFD, 0xFD, 0xFF)
      };
      break;
    case ColorSetId_Dark:
      currTheme = (theme_t) {
        .textColor            =      RGBA8(0xFF, 0xFF, 0xFF, 0xFF),
        .backgroundColor      =      RGBA8(0x31, 0x31, 0x31, 0xFF),
        .highlightColor       =      RGBA8(0x27, 0xA3, 0xC7, 0xFF),
        .selectedColor        =      RGBA8(0x59, 0xED, 0xC0, 0xFF),
        .separatorColor       =      RGBA8(0x60, 0x60, 0x60, 0x80),
        .selectedButtonColor  =      RGBA8(0x25, 0x26, 0x2A, 0xFF)
      };
      break;
  }
}
