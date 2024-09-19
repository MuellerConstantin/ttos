#include <io/stream.h>
#include <memory/kheap.h>

char* stream_readline(void (*putchar)(char), char (*getchar)(void), bool echo) {
    char *buffer = kmalloc(1);
    size_t buffer_size = 1;
    size_t buffer_index = 0;

    while(true) {
        char ch = getchar();

        if(ch == '\n') {
            putchar(ch);
            break;
        }

        if(ch == '\b') {
            if(buffer_index == 0) {
                continue;
            }

            buffer_index--;
            putchar(ch);

            continue;
        }

        if(echo) {
            putchar(ch);
        }

        if(buffer_index == buffer_size) {
            buffer_size *= 2;
            buffer = krealloc(buffer, buffer_size);
        }

        buffer[buffer_index] = ch;
        buffer_index++;
    }

    buffer[buffer_index] = '\0';

    return buffer;
}

int stream_printf(void (*putchar)(char), const char *format, ...) {
    va_list args;
    va_start(args, format);
    int value = stream_vprintf(putchar, format, args);
    va_end(args);

    return value;
}

int stream_vprintf(void (*putchar)(char), const char *format, va_list args) {
    int count = 0;

    while(*format != '\0') {
        if(*format == '%') {
            format++;

            switch(*format) {
                case 'c': {
                    char ch = va_arg(args, int);
                    putchar(ch);
                    count++;
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, const char*);

                    while(*str != '\0') {
                        putchar(*str);
                        str++;
                        count++;
                    }

                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    itoa(num, num_str, 10);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        putchar(num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    putchar('0');
                    putchar('x');

                    itoa(num, num_str, 16);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        putchar(num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    char ptr_str[32];

                    itoa((uintptr_t) ptr, ptr_str, 16);

                    for(int i = 0; ptr_str[i] != '\0'; i++) {
                        putchar(ptr_str[i]);
                        count++;
                    }

                    break;
                }
                case 'b': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    putchar('0');
                    putchar('b');

                    itoa(num, num_str, 2);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        putchar(num_str[i]);
                        count++;
                    }

                    break;
                }
                default: {
                    break;
                }
            }
        } else {
            putchar(*format);
            count++;
        }

        format++;
    }

    return count;
}
