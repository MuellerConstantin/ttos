#ifndef _KERNEL_UTIL_NUMERIC_H
#define _KERNEL_UTIL_NUMERIC_H

#include <stdint.h>

/**
 * Converts an integer to a string.
 * 
 * @param n The integer to convert.
 * @param buf The buffer to store the string in.
 * @param base The base to convert the integer to.
 * @return The string representation of the integer.
 */
char *itoa(int32_t n, char *buf, uint32_t base);

/**
 * Converts a double to a string.
 * 
 * @param n The double to convert.
 * @param precision The number of decimal places to include.
 * @param buf The buffer to store the string in.
 * @return The string representation of the double.
 */
char *gcvt(double n, int precision, char *buf);

/**
 * Computes the smallest integer value not less than `x`.
 * 
 * @param x The value to compute the ceiling of.
 * @return The smallest integer value not less than `x`.
 */
float ceil(float x);

#endif // _KERNEL_UTIL_NUMERIC_H
