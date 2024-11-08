#include <util/numeric.h>
#include <util/string.h>
#include <stdbool.h>

static char ITOA_CHARS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
    'U', 'V', 'W', 'X', 'Y', 'Z'};

char *itoa(int32_t n, char *buf, uint32_t base) {
    char *ptr = buf;
    bool is_negative = false;

    if (base < 2 || base > 32) {
        *ptr = '\0';
        return buf;
    } else if (n == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return buf;
    }

    if (n < 0 && base == 10) {
        is_negative = true;
        n = -n;
    }

    while (n != 0) {
        *ptr++ = ITOA_CHARS[n % base];
        n /= base;
    }

    if (is_negative) {
        *ptr++ = '-';
    }

    *ptr = '\0';

    return strrev(buf);
}

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

float ceil(float x) {
    unsigned int input;
    memcpy(&input, &x, 4);

    // Small numbers get rounded to 0 or 1, depending on their sign

    int exponent = ((input >> 23) & 255) - 127;

    if (exponent < 0) {
        return (x > 0);
    }

    // numbers without fractional bits are mapped to themselves

    int fractional_bits = 23 - exponent;

    if (fractional_bits <= 0) {
        return x;
    }

    // Round the number down by masking out the fractional bits

    unsigned int integral_mask = 0xffffffff << fractional_bits;
    unsigned int output = input & integral_mask;

    // Positive numbers need to be rounded up, not down

    memcpy(&x, &output, 4);

    if (x > 0 && output != input) {
        ++x;
    }

    return x;
}
