#ifndef _STDIO_H
#define _STDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

#define O_RDONLY    0b00000001
#define O_WRONLY    0b00000010
#define O_RDWR      (O_RDONLY | O_WRONLY)
#define O_CREAT     0b00000100
#define O_TRUNC     0b00001000
#define O_APPEND    0x00010000

struct _FILE {
    int32_t fd;
    uint32_t flags;
};

typedef struct _FILE FILE;

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

/**
 * Reads a string from the standard input stream until a newline is read.
 * 
 * @param str The string buffer to read into.
 * @return The string or NULL on error.
 */
char *gets(char *str);

/**
 * Opens a file.
 * 
 * @param filename The name of the file to open.
 * @param mode The mode to open the file in.
 * @return The opened file or NULL on error.
 */
FILE* fopen(const char * filename, const char* mode);

/**
 * Closes a file.
 * 
 * @param stream The file to close.
 * @return 0 on success or EOF on error.
 */
int fclose(FILE* stream);

/**
 * Reads data from a file.
 * 
 * @param buffer The buffer to read into.
 * @param size The size of the buffer.
 * @param count The number of bytes to read.
 * @param stream The file to read from.
 * @return The number of bytes read or EOF on error.
 */
size_t fread(void* buffer, size_t size, size_t count, FILE* stream);

#ifdef __cplusplus
}
#endif

#endif // _STDIO_H
