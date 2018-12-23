#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <threads.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <math.h>
#ifdef __SWITCH__
#include <switch.h>
#endif

#include <stdint.h>
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u32 Result;

typedef void (*workerThreadFunc)(void *);

#ifdef _WIN32
#define DIRECTORY_SEPARATOR_CHAR '\\'
static const char DIRECTORY_SEPARATOR[] = "\\";
#else
#define DIRECTORY_SEPARATOR_CHAR '/'
static const char DIRECTORY_SEPARATOR[] = "/";
#endif


#define M_TAU (2*M_PI)

typedef union {
    uint32_t abgr;
    struct {
        uint8_t r,g,b,a;
    };
} color_t;

// when building for pc we need to include these separately
#ifndef __SWITCH__
#include "switch/nro.h"
#include "switch/nacp.h"
#endif

#include "font.h"
#include "menu.h"
#include "text.h"
#include "ui.h"
#include "assets.h"
#include "launch.h"
#include "worker.h"
#include <turbojpeg.h>
#include "math.h"
#include "theme.h"
#include "message-box.h"
#include "power.h"
#include "netloader.h"

void menuStartupPath(void);
void menuStartup(void);
void themeMenuStartup(void);
void menuLoop(void);

static inline uint8_t BlendColor(uint32_t src, uint32_t dst, uint8_t alpha)
{
    uint8_t one_minus_alpha = (uint8_t)255 - alpha;
    return (dst*alpha + src*one_minus_alpha)/(uint8_t)255;
}

static inline color_t MakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    color_t clr;
    clr.r = r;
    clr.g = g;
    clr.b = b;
    clr.a = a;
    return clr;
}

#ifdef __SWITCH__
extern uint8_t* g_framebuf;
extern u32 g_framebuf_width;
static inline void DrawPixel(uint32_t x, uint32_t y, color_t clr)
{
    if (x >= 1280 || y >= 720)
        return;
    u32 off = (y * g_framebuf_width + x)*4;
    g_framebuf[off] = BlendColor(g_framebuf[off], clr.r, clr.a); off++;
    g_framebuf[off] = BlendColor(g_framebuf[off], clr.g, clr.a); off++;
    g_framebuf[off] = BlendColor(g_framebuf[off], clr.b, clr.a); off++;
    g_framebuf[off] = 0xff;
}
static inline void DrawPixelRaw(uint32_t x, uint32_t y, color_t clr)
{
    if (x >= 1280 || y >= 720)
        return;
    u32 off = (y * g_framebuf_width + x)*4;
    *((u32*)&g_framebuf[off]) = clr.r | (clr.g<<8) | (clr.b<<16) | (0xff<<24);
}
static inline void Draw4PixelsRaw(uint32_t x, uint32_t y, color_t clr)
{
    if (x >= 1280 || y >= 720 || x > 1280-4)
        return;

    u32 color = clr.r | (clr.g<<8) | (clr.b<<16) | (0xff<<24);
    u128 val = color | ((u128)color<<32) | ((u128)color<<64) | ((u128)color<<96);
    u32 off = (y * g_framebuf_width + x)*4;
    *((u128*)&g_framebuf[off]) = val;
}
static inline color_t FetchPixelColor(uint32_t x, uint32_t y)
{
    u32 off = (y * g_framebuf_width + x)*4;
    u32 val = *((u32*)&g_framebuf[off]);
    u8 r = (u8)val;
    u8 g = (u8)(val>>8);
    u8 b = (u8)(val>>16);
    return MakeColor(r, g, b, 255);
}
#else
extern color_t pixels[720][1280];
static inline void DrawPixel(uint32_t x, uint32_t y, color_t clr)
{
    if (x >= 1280 || y >= 720)
        return;
    pixels[y][x].r = BlendColor(pixels[y][x].r, clr.r, clr.a);
    pixels[y][x].g = BlendColor(pixels[y][x].g, clr.g, clr.a);
    pixels[y][x].b = BlendColor(pixels[y][x].b, clr.b, clr.a);
    pixels[y][x].a = 0xff;
}
static inline void DrawPixelRaw(uint32_t x, uint32_t y, color_t clr)
{
    if (x >= 1280 || y >= 720)
        return;
    pixels[y][x].r = clr.r;
    pixels[y][x].g = clr.g;
    pixels[y][x].b = clr.b;
    pixels[y][x].a = 0xff;
}
static inline void Draw4PixelsRaw(uint32_t x, uint32_t y, color_t clr)
{
    uint32_t pos;
    for (pos=0; pos<4; pos++) DrawPixelRaw(x+pos, y, clr);
}
static inline color_t FetchPixelColor(uint32_t x, uint32_t y)
{
    return pixels[y][x];
}
#endif

void DrawPixel(uint32_t x, uint32_t y, color_t clr);
void DrawText(u32 font, uint32_t x, uint32_t y, color_t clr, const char* text);
void DrawTextTruncate(u32 font, uint32_t x, uint32_t y, color_t clr, const char* text, uint32_t max_width, const char* end_text);
void GetTextDimensions(u32 font, const char* text, uint32_t* width_out, uint32_t* height_out);
uint32_t GetTextXCoordinate(u32 font, uint32_t rX, const char* text, const char align);
uint32_t GetTextYCoordinate(u32 font, uint32_t rY, const char* text, const char align);

bool fontInitialize(void);
void fontExit();
