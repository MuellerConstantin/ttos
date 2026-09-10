#include <stdlib.h>
#include <string.h>

static char SIZETOA_UNITS[] = {'B', 'K', 'M', 'G'};

char *sizetoa(uint32_t n, char *buf) {
    uint32_t unit = 0;
    uint32_t remainder = 0;

    while(n >= 1024 && unit < sizeof(SIZETOA_UNITS) - 1) {
        remainder = n % 1024;
        n /= 1024;
        unit++;
    }

    itoa((int32_t) n, buf, 10);

    /*
     * Below ten the integer part alone is too coarse to be useful, so a single
     * decimal place is appended. It is derived from the remainder of the last
     * division rather than from a division of its own, which keeps the whole
     * conversion free of floating point.
     */
    if(unit > 0 && n < 10) {
        uint32_t decimal = (remainder * 10) / 1024;

        char* end = buf + strlen(buf);

        *end++ = '.';
        *end++ = (char) ('0' + decimal);
        *end = '\0';
    }

    char* end = buf + strlen(buf);

    *end++ = SIZETOA_UNITS[unit];
    *end = '\0';

    return buf;
}
