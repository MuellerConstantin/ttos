#ifndef _STRING_H
#define _STRING_H

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

#ifdef __cplusplus
}
#endif

#endif // _STRING_H
