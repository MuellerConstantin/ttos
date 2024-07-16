#include <string.h>

int strncmp(const char *str1, const char *str2, size_t n) {
    while (n--) {
        if (*str1 != *str2) {
            return (*str1 - *str2);
        }

        if (*str1 == '\0') {
            return 0;
        }

        str1++;
        str2++;
    }

    return 0;
}
