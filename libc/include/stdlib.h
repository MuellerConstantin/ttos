#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>
#include <stddef.h>

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

/**
 * Allocates memory.
 * 
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory or NULL if allocation failed.
 */
void* malloc(size_t size);

/**
 * Allocate memory for an array of elements and set the memory to zero.
 * 
 * @param num The number of elements.
 * @param size The size of each element.
 * @return The address of the allocated block of memory or NULL if allocation failed.
 */
void* calloc(size_t num, size_t size);

/**
 * Reallocate a block of memory with a new size.
 * 
 * @param ptr The address of the block to reallocate.
 * @param size The new size of the block.
 * @return The address of the reallocated block.
 */
void* realloc(void* ptr, size_t size);

/**
 * Free a block of memory allocated by malloc.
 * 
 * @param ptr The address of the block to free.
 */
void free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // _STDLIB_H
