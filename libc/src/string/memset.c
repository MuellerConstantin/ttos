#include <string.h>

void *memset(void *dest, uint8_t ch, size_t n) {
    uint8_t *ptr = (uint8_t *) dest;
    uint8_t *end = ptr + n;

    while(ptr != end) {
        *ptr++ = ch;
    }

    return dest;
}

void *memsetw(void *dest, uint16_t value, size_t n) {
    uint16_t *ptr = (uint16_t *) dest;
    uint16_t *end = ptr + n;

    while(ptr != end) {
        *ptr++ = value;
    }

    return dest;
}