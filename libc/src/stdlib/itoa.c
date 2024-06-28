#include <stdlib.h>
#include <string.h>

static char ITOA_CHARS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
    'U', 'V', 'W', 'X', 'Y', 'Z'};

char *itoa(uint32_t n, char *buf, uint32_t base) {
    char *ptr = buf;

    if (base < 2 || base > 32) {
        *ptr = '\0';
        return buf;
    } else if (n == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return buf;
    }

    while (n != 0) {
        *ptr++ = ITOA_CHARS[n % base];
        n /= base;
    }

    *ptr = '\0';

    return strrev(buf);
}
