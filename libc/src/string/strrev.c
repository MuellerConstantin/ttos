#include <string.h>

char *strrev(char *str) {
    size_t len = strlen(str);

    char *ptr1 = str;
    char *ptr2 = str + (len - 1);

    while(ptr1 < ptr2) {
        char c = *ptr1;
        *ptr1++ = *ptr2;
        *ptr2-- = c;
    }
    
    return str;
}
