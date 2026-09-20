#ifndef _KERNEL_IO_STREAM_H
#define _KERNEL_IO_STREAM_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

typedef struct stream stream_t;

struct stream {
    void (*putchar)(stream_t* stream, char ch);
    void (*puts)(stream_t* stream, const char* str);
    int32_t (*read)(stream_t* stream, char* buffer, size_t size);
    int32_t (*write)(stream_t* stream, const char* buffer, size_t size);
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
 * Reads from the stream into a buffer, blocking until at least one byte is
 * available.
 *
 * @param stream The stream.
 * @param buffer The buffer to fill.
 * @param size The size of the buffer.
 * @return The number of bytes read, or -1 if the caller may not read from
 *         this stream.
 */
int32_t stream_read(stream_t* stream, char* buffer, size_t size);

/**
 * Prints a string to the stream.
 * 
 * @param stream The stream.
 * @param str The string to print.
 */
void stream_puts(stream_t* stream, const char* str);

/**
 * Writes a buffer to the stream. Unlike stream_puts the data is not a string:
 * it may contain NUL bytes and carries its own length.
 *
 * @param stream The stream.
 * @param buffer The bytes to write.
 * @param size The number of bytes.
 * @return The number of bytes written, or -1 if the caller may not write to
 *         this stream.
 */
int32_t stream_write(stream_t* stream, const char* buffer, size_t size);

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
