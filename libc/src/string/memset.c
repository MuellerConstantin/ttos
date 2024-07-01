#include <string.h>

void *memset(void *dest, uint8_t ch, size_t n) {
    uint8_t *ptr = (uint8_t *) dest;
    uint8_t *end = ptr + n;

    while(ptr != end) {
        *ptr++ = ch;
    }

    return dest;
}
