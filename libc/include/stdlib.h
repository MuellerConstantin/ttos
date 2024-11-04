#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAND_MAX 32767

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
 * Initializes the random number generator with a seed.
 * 
 * @param seed The seed to initialize the random number generator with.
 */
void srand(unsigned int seed);

/**
 * Generates a random number.
 * 
 * @return A random number between 0 and RAND_MAX.
 */
int rand(void);

#ifdef __cplusplus
}
#endif

#endif // _STDLIB_H
