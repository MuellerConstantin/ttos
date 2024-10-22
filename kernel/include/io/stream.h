#ifndef _KERNEL_IO_STREAM_H
#define _KERNEL_IO_STREAM_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

typedef struct stream stream_t;

struct stream {
    void (*putchar)(stream_t* stream, char ch);
    char (*getchar)(stream_t* stream);
    void (*puts)(stream_t* stream, const char* str);
    char* (*gets)(stream_t* stream);
    void* data;
};

/**
 * Prints a character to the stream.
 * 
 * @param stream The stream.
 * @param ch The character to print.
 */
void stream_putchar(stream_t* stream, char ch);

/**
 * Reads a character from the stream.
 * 
 * @param stream The stream.
 * @return The character read.
 */
char stream_getchar(stream_t* stream);

/**
 * Prints a string to the stream.
 * 
 * @param stream The stream.
 * @param str The string to print.
 */
void stream_puts(stream_t* stream, const char* str);

/**
 * Reads a line from the stream.
 * 
 * @param stream The stream.
 * @return The line read.
 */
char* stream_gets(stream_t* stream);

/**
 * Naive printf implementation that writes to a stream. This function does not
 * support all the features of the standard printf function.
 * 
 * @param stream The stream.
 * @param format The format string.
 * @param ... The arguments to format.
 * @return The number of characters written.
 */
int stream_printf(stream_t* stream, const char *format, ...);

/**
 * Naive vprintf implementation that writes to a stream. This function does not
 * support all the features of the standard vprintf function.
 * 
 * @param stream The stream.
 * @param format The format string.
 * @param args The arguments to format.
 * @return The number of characters written.
 */
int stream_vprintf(stream_t* stream, const char *format, va_list args);

#endif // _KERNEL_IO_STREAM_H
