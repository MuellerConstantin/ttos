#include <io/stream.h>

void stream_putchar(stream_t* stream, char ch) {
    stream->putchar(stream, ch);
}

int32_t stream_read(stream_t* stream, char* buffer, size_t size) {
    return stream->read(stream, buffer, size);
}

void stream_puts(stream_t* stream, const char* str) {
    stream->puts(stream, str);
}

int32_t stream_write(stream_t* stream, const char* buffer, size_t size) {
    return stream->write(stream, buffer, size);
}

int stream_printf(stream_t* stream, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int value = stream_vprintf(stream, format, args);
    va_end(args);

    return value;
}

int stream_vprintf(stream_t* stream, const char *format, va_list args) {
    int count = 0;

    while(*format != '\0') {
        if(*format == '%') {
            format++;

            switch(*format) {
                case '%': {
                    stream_putchar(stream, '%');
                    count++;
                    break;
                }
                case 'c': {
                    char ch = va_arg(args, int);
                    stream_putchar(stream, ch);
                    count++;
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, const char*);

                    while(*str != '\0') {
                        stream_putchar(stream, *str);
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
                        stream_putchar(stream, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    stream_putchar(stream, '0');
                    stream_putchar(stream, 'x');

                    itoa(num, num_str, 16);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        stream_putchar(stream, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    char ptr_str[32];

                    itoa((uintptr_t) ptr, ptr_str, 16);

                    for(int i = 0; ptr_str[i] != '\0'; i++) {
                        stream_putchar(stream, ptr_str[i]);
                        count++;
                    }

                    break;
                }
                case 'b': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    stream_putchar(stream, '0');
                    stream_putchar(stream, 'b');

                    itoa(num, num_str, 2);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        stream_putchar(stream, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'f': {
                    double num = va_arg(args, double);
                    char num_str[32];

                    gcvt(num, 2, num_str);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        stream_putchar(stream, num_str[i]);
                        count++;
                    }

                    break;
                }
                default: {
                    break;
                }
            }
        } else {
            stream_putchar(stream, *format);
            count++;
        }

        format++;
    }

    return count;
}
