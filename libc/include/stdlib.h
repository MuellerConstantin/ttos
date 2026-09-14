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
 * Converts a number of bytes to a string with a binary unit suffix, such as
 * "512B", "1.5K" or "49M". Values below ten carry a single decimal place.
 *
 * @param n The number of bytes to convert.
 * @param buf The buffer to store the string in, at least 8 bytes long.
 * @return The string representation of the size.
 */
char *sizetoa(uint32_t n, char *buf);

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

/**
 * Converts the leading number of a text. Whitespace in front is skipped, a
 * sign is honoured the way the standard does it, by negating the unsigned
 * result. Base 0 reads a 0x prefix as hexadecimal and a leading 0 as octal;
 * base 16 accepts the 0x prefix as well. A number too large for the result is
 * reported as the largest value there is. There is no errno to say so.
 * 
 * @param text The text to convert.
 * @param end Where to store a pointer to the first character not converted, or
 *            to the text itself when no digit was found. May be NULL.
 * @param base The base, 0 or 2 to 36.
 * @return The value, 0 if there was no number or the base is invalid.
 */
unsigned long strtoul(const char* text, char** end, int base);

/**
 * Looks up a variable in the environment of the process.
 * 
 * @param name The name of the variable.
 * @return A pointer to the value inside the environment, or NULL if the variable is not set.
 *         The pointer is only valid until the variable is changed.
 */
char* getenv(const char* name);

/**
 * Sets a variable in the environment of the process. The environment is copied to the heap
 * the first time it grows; a value that is replaced is not freed, as it may still be
 * referenced by a pointer getenv handed out.
 * 
 * @param name The name of the variable, non-empty and without '='.
 * @param value The value to set.
 * @param overwrite Whether an existing variable is replaced. If zero and the variable exists,
 *                  the call succeeds without changing anything.
 * @return 0 on success or -1 if the name is invalid or memory ran out.
 */
int setenv(const char* name, const char* value, int overwrite);

#ifdef __cplusplus
}
#endif

#endif // _STDLIB_H
