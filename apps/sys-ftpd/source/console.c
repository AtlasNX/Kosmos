#define ENABLE_LOGGING 1
#include "console.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this is a lot easier when you have a real console */

int should_log = 0;

void
console_init(void)
{
}

void
console_set_status(const char *fmt, ...)
{

}

void
console_print(const char *fmt, ...)
{
  if(should_log) {
    stdout = stderr = fopen("/logs/ftpd.log", "a");
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    fclose(stdout);
  }
}

void
debug_print(const char *fmt, ...)
{
  if(should_log) {
    stdout = stderr = fopen("/logs/ftpd.log", "a");
#ifdef ENABLE_LOGGING
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
#endif
    fclose(stdout);
  }
}

void console_render(void)
{
}
