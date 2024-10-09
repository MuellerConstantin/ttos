#include <string.h>

int memcmp(const void *s1, const void *s2, size_t n) {

    const uint8_t *byte1 = (const uint8_t *) s1;
    const uint8_t *byte2 = (const uint8_t *) s2;

    while((*byte1 == *byte2) && (n > 0)) {

        ++byte1;
        ++byte2;
        --n;
    }

    if (n == 0) {

        return 0;
    }
	
    return *byte1 - *byte2;
}
