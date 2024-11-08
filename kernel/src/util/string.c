#include <util/string.h>
#include <stdarg.h>

static int vstrfmt(char * str, const char *format, va_list args);

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

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    while(n--) {
        *d++ = *s++;
    }
    return dest;
}

void *memset(void *dest, uint8_t ch, size_t n) {
    uint8_t *ptr = (uint8_t *) dest;
    uint8_t *end = ptr + n;

    while(ptr != end) {
        *ptr++ = ch;
    }

    return dest;
}

void *memsetw(void *dest, uint16_t value, size_t n) {
    uint16_t *ptr = (uint16_t *) dest;
    uint16_t *end = ptr + n;

    while(ptr != end) {
        *ptr++ = value;
    }

    return dest;
}

char* strcat(char* dest, const char* src) {
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);

    for(size_t i = 0; i < src_len; i++) {
        dest[dest_len + i] = src[i];
    }

    dest[dest_len + src_len] = '\0';

    return dest;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && *str1 == *str2) {
        str1++;
        str2++;
    }

    return *(unsigned char*) str1 - *(unsigned char*) str2;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    const char* s = src;

    while ((*d++ = *s++));

    return dest;
}

size_t strlen(const char *str) {
	size_t len = 0;

	while(str && *str != '\0') {
		++str;
		++len;
	}

	return len;
}

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

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;

    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }

    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}

char *strpbrk(const char *s, const char *accept) {
    while (*s != '\0') {
        const char *a = accept;
        while (*a != '\0') {
            if (*s == *a) {
                return (char *)s;
            }
            a++;
        }
        s++;
    }

    return NULL;
}

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

char *strsep(char **stringp, const char *delim) {
    char *start = *stringp;
    char *p;

    if (start == NULL) {
        return NULL;
    }

    p = strpbrk(start, delim);

    if (p == NULL) {
        *stringp = NULL;
    } else {
        *p = '\0';
        *stringp = p + 1;
    }

    return start;
}

int strfmt(char * str, const char * format, ... ) {
    va_list args;
    va_start(args, format);
    int ret = vstrfmt(str, format, args);
    va_end(args);
    return ret;
}

static int vstrfmt(char* str, const char *format, va_list args) {
    int count = 0;

    while(*format != '\0') {
        if(*format == '%') {
            format++;

            switch(*format) {
                case '%': {
                    str[count] = '%';
                    count++;
                    break;
                }
                case 'c': {
                    char ch = va_arg(args, int);
                    str[count] = ch;
                    count++;
                    break;
                }
                case 's': {
                    const char *placement_str = va_arg(args, const char*);

                    while(*placement_str != '\0') {
                        str[count] = *placement_str;
                        placement_str++;
                        count++;
                    }

                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    itoa(num, num_str, 10);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        str[count] = num_str[i];
                        count++;
                    }

                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    str[count++] = '0';
                    str[count++] = 'x';

                    itoa(num, num_str, 16);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        str[count] = num_str[i];
                        count++;
                    }

                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    char ptr_str[32];

                    itoa((uintptr_t) ptr, ptr_str, 16);

                    for(int i = 0; ptr_str[i] != '\0'; i++) {
                        str[count] = ptr_str[i];
                        count++;
                    }

                    break;
                }
                case 'b': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    str[count++] = '0';
                    str[count++] = 'b';

                    itoa(num, num_str, 2);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        str[count] = num_str[i];
                        count++;
                    }

                    break;
                }
                case 'f': {
                    double num = va_arg(args, double);
                    char num_str[32];

                    gcvt(num, 2, num_str);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        str[count] = num_str[i];
                        count++;
                    }

                    break;
                }
                default: {
                    break;
                }
            }
        } else {
            str[count] = *format;
            count++;
        }

        format++;
    }

    str[count] = '\0';

    return count;
}
