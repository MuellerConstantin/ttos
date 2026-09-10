#ifndef _LIBC_FORMAT_H
#define _LIBC_FORMAT_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct format_sink format_sink_t;

/**
 * Destination the formatting core writes its characters to. printf and sprintf
 * differ only in where the characters end up, so the format string is
 * interpreted in one place and the destination is passed in as a sink.
 */
struct format_sink {
    /**
     * Writes a single character to the destination. The core keeps the count
     * itself, so a sink writing into a buffer can use it as the write index.
     */
    void (*write)(format_sink_t* sink, char ch);

    char* buffer;
    int count;
};

/**
 * Interprets a format string and writes the result to a sink. This is an
 * implementation detail of the printf family rather than something programs are
 * meant to call directly.
 *
 * @param sink The destination to write to.
 * @param format The format string.
 * @param args The arguments to format.
 * @return The number of characters written.
 */
int format_write(format_sink_t* sink, const char* format, va_list args);

#ifdef __cplusplus
}
#endif

#endif // _LIBC_FORMAT_H
