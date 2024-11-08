#include <util/random.h>

static unsigned long next = 1;

void random_seed(unsigned int seed) {
    next = seed;
}

int random_next(void) {
    next = next * 1103515245 + 12345;
    return (unsigned int) (next / 65536) % 32768;
}

