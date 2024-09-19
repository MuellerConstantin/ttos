#ifndef _KERNEL_IO_STREAM_H
#define _KERNEL_IO_STREAM_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Reads a line from a stream.
 * 
 * @param putchar The function to write a character to the stream, used for echo.
 * @param getchar The function to read a character from the stream.
 * @param echo Whether to echo the input.
 * @return The line read from the stream.
 */
char* stream_readline(void (*putchar)(char), char (*getchar)(void), bool echo);

/**
 * Naive printf implementation that writes to a stream. This function does not
 * support all the features of the standard printf function.
 * 
 * @param putchar The function to write a character to the stream.
 * @param format The format string.
 * @param ... The arguments to format.
 * @return The number of characters written.
 */
int stream_printf(void (*putchar)(char), const char *format, ...);

/**
 * Naive vprintf implementation that writes to a stream. This function does not
 * support all the features of the standard vprintf function.
 * 
 * @param putchar The function to write a character to the stream.
 * @param format The format string.
 * @param args The arguments to format.
 * @return The number of characters written.
 */
int stream_vprintf(void (*putchar)(char), const char *format, va_list args);

#endif // _KERNEL_IO_STREAM_H
