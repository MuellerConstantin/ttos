#ifndef _KERNEL_UTIL_RANDOM_H
#define _KERNEL_UTIL_RANDOM_H

#define RAND_MAX 32767

/**
 * Initializes the random number generator with a seed.
 * 
 * @param seed The seed to initialize the random number generator with.
 */
void random_seed(unsigned int seed);

/**
 * Generates a random number.
 * 
 * @return A random number between 0 and RAND_MAX.
 */
int random_next(void);

#endif // _KERNEL_UTIL_RANDOM_H
