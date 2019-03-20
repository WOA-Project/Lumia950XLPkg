#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stdint.h>

int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int snprintf(char *str, size_t size, const char *format, ...);

int vprintf(const char *format, va_list ap);
int printf(const char *format, ...);

#endif
