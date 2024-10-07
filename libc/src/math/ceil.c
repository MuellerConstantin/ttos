#include <math.h>
#include <string.h>

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
