#ifndef _STDIO_H
#define _STDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Naive sprintf implementation that writes to a string buffer. This function does not
 * support all the features of the standard sprintf function.
 * 
 * @param str The string buffer to write to.
 * @param format The format string.
 * @param ... The arguments to format.
 * @return The number of characters written.
 */
int sprintf(char * str, const char * format, ... );

#ifdef __cplusplus
}
#endif

#endif // _STDIO_H
