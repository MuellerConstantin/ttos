#include <stdlib.h>
#include <string.h>

char *gcvt(double n, int precision, char *buf) {
    char *ptr = buf;

    int whole_part = (int) n;
    float fraction_part = n - whole_part;

    itoa(whole_part, ptr, 10);
    ptr += strlen(ptr);

    *ptr++ = '.';

    for (int i = 0; i < precision; i++) {
        fraction_part *= 10;
        int digit = (int) fraction_part;
        *ptr++ = '0' + digit;
        fraction_part -= digit;
    }

    *ptr = '\0';

    return buf;
}
