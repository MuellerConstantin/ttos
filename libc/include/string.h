#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Determines the length of a string.
 * 
 * @param str The string to determine the length of.
 * @return The length of the string.
 */
size_t strlen(const char*);

/**
 * Reverses a string.
 * 
 * @param str The string to reverse.
 * @return The reversed string.
 */
char *strrev(char *str);

/**
 * Copies the given value into each of the first n characters
 * of the object pointed to by dest.
 * 
 * @param dest The destination to copy the value to.
 * @param ch The value to copy.
 * @param n The number of characters to copy.
 * @return A pointer to the destination.
 */
void *memset(void *dest, uint8_t ch, size_t n);

#ifdef __cplusplus
}
#endif

#endif // _STRING_H
