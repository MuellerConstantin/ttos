#include <string.h>

char* strcat(char* dest, const char* src) {
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);

    for(size_t i = 0; i < src_len; i++) {
        dest[dest_len + i] = src[i];
    }

    dest[dest_len + src_len] = '\0';

    return dest;
}
