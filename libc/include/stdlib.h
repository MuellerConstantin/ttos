#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Converts an integer to a string.
 * 
 * @param n The integer to convert.
 * @param buf The buffer to store the string in.
 * @param base The base to convert the integer to.
 * @return The string representation of the integer.
 */
char *itoa(uint32_t n, char *buf, uint32_t base);

#ifdef __cplusplus
}
#endif

#endif // _STDLIB_H
