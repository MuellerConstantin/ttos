#include <stdio.h>

static int vsprintf(char * str, const char *format, va_list args);

int sprintf(char * str, const char * format, ... ) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(str, format, args);
    va_end(args);
    return ret;
}

static int vsprintf(char* str, const char *format, va_list args) {
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
