#include <string.h>

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    const char* s = src;

    while ((*d++ = *s++));

    return dest;
}
