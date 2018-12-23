#pragma once

#ifdef _3DS
#include <3ds.h>
#define ESC(x) "\x1b[" #x
#define RESET   ESC(0m)
#define BLACK   ESC(30m)
#define RED     ESC(31;1m)
#define GREEN   ESC(32;1m)
#define YELLOW  ESC(33;1m)
#define BLUE    ESC(34;1m)
#define MAGENTA ESC(35;1m)
#define CYAN    ESC(36;1m)
#define WHITE   ESC(37;1m)
#else
#define ESC(x)
#define RESET
#define BLACK
#define RED
#define GREEN
#define YELLOW
#define BLUE
#define MAGENTA
#define CYAN
#define WHITE
#endif

extern int should_log;

void console_init(void);

__attribute__((format(printf,1,2)))
void console_set_status(const char *fmt, ...);

__attribute__((format(printf,1,2)))
void console_print(const char *fmt, ...);

__attribute__((format(printf,1,2)))
void debug_print(const char *fmt, ...);

void console_render(void);