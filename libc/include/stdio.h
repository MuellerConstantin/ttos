#ifndef _STDIO_H
#define _STDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define EOF (-1)

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
 * @return The number of characters written or EOF on error.
 */
int sprintf(char* str, const char * format, ... );

/**
 * Writes a string to the standard output stream.
 * 
 * @param str The string to write.
 * @return The number of characters written or EOF on error.
 */
int puts(const char* str);

/**
 * Writes a character to the standard output stream.
 * 
 * @param ch The character to write.
 * @return The written character or EOF on error.
 */
int putchar(int ch);

/**
 * Writes a formatted string to the standard output stream. This function does not
 * support all the features of the standard printf function.
 *
 * @param format The format string.
 * @param ... The arguments to format.
 * @return The number of characters written or EOF on error.
 */
int printf(const char *format, ... );

/**
 * Reads a character from the standard input stream.
 * 
 * @return The read character or EOF on error.
 */
int getchar(void);

#ifdef __cplusplus
}
#endif

#endif // _STDIO_H
