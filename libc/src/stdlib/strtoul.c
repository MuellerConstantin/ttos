#include <stdlib.h>

/** Largest value the result can hold; a number beyond it is reported as this. */
#define STRTOUL_MAX 0xFFFFFFFFUL

/** The value a character stands for in a given base, or the base itself if it stands for none. */
static unsigned long strtoul_digit(char character, unsigned long base) {
    unsigned long value;

    if (character >= '0' && character <= '9') {
        value = (unsigned long) (character - '0');
    } else if (character >= 'a' && character <= 'z') {
        value = (unsigned long) (character - 'a') + 10;
    } else if (character >= 'A' && character <= 'Z') {
        value = (unsigned long) (character - 'A') + 10;
    } else {
        return base;
    }

    return value < base ? value : base;
}

unsigned long strtoul(const char* text, char** end, int base) {
    const char* cursor = text;
    int negative = 0;

    if (base < 0 || base == 1 || base > 36) {
        if (end) {
            *end = (char*) text;
        }

        return 0;
    }

    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n') {
        cursor++;
    }

    if (*cursor == '-' || *cursor == '+') {
        negative = *cursor == '-';
        cursor++;
    }

    /*
     * A 0x prefix is accepted for base 16 and selects it for base 0; a leading 0
     * alone selects base 8 for base 0. The prefix only counts when a digit follows,
     * otherwise the 0 is the whole number.
     */
    if ((base == 0 || base == 16) && cursor[0] == '0' && (cursor[1] == 'x' || cursor[1] == 'X') && strtoul_digit(cursor[2], 16) < 16) {
        base = 16;
        cursor += 2;
    } else if (base == 0) {
        base = cursor[0] == '0' ? 8 : 10;
    }

    unsigned long value = 0;
    unsigned long limit = STRTOUL_MAX / (unsigned long) base;
    int digits = 0;
    int overflow = 0;

    for (;; cursor++) {
        unsigned long digit = strtoul_digit(*cursor, (unsigned long) base);

        if (digit >= (unsigned long) base) {
            break;
        }

        // Past this point the next digit would not fit; the rest is still consumed.
        if (value > limit || value * (unsigned long) base > STRTOUL_MAX - digit) {
            overflow = 1;
        } else {
            value = value * (unsigned long) base + digit;
        }

        digits++;
    }

    // Without a single digit nothing was converted and the whole text stays unread.
    if (end) {
        *end = (char*) (digits > 0 ? cursor : text);
    }

    if (overflow) {
        return STRTOUL_MAX;
    }

    return negative ? (unsigned long) (0 - value) : value;
}
