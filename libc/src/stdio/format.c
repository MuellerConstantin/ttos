#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <format.h>

// Long enough for a 32 bit value in base 2 plus a sign and a terminator.
#define FORMAT_BODY_LENGTH 34

static void format_emit(format_sink_t* sink, char ch);
static void format_repeat(format_sink_t* sink, char ch, int times);
static void format_string(format_sink_t* sink, const char* str);
static void format_padded(format_sink_t* sink, const char* prefix, const char* body, int width, bool left, bool zero);

static void format_emit(format_sink_t* sink, char ch) {
    sink->write(sink, ch);
    sink->count++;
}

static void format_repeat(format_sink_t* sink, char ch, int times) {
    for(int i = 0; i < times; i++) {
        format_emit(sink, ch);
    }
}

static void format_string(format_sink_t* sink, const char* str) {
    while(*str != '\0') {
        format_emit(sink, *str);
        str++;
    }
}

/*
 * Writes prefix and body padded out to the requested width. The prefix is kept
 * apart from the body so that zero padding lands behind a sign or a base
 * marker rather than in front of it, which would turn "0x1f" into "000x1f".
 */
static void format_padded(format_sink_t* sink, const char* prefix, const char* body, int width, bool left, bool zero) {
    int padding = width - (int) strlen(prefix) - (int) strlen(body);

    if(padding < 0) {
        padding = 0;
    }

    if(left) {
        format_string(sink, prefix);
        format_string(sink, body);
        format_repeat(sink, ' ', padding);

        return;
    }

    if(zero) {
        format_string(sink, prefix);
        format_repeat(sink, '0', padding);
        format_string(sink, body);

        return;
    }

    format_repeat(sink, ' ', padding);
    format_string(sink, prefix);
    format_string(sink, body);
}

int format_write(format_sink_t* sink, const char* format, va_list args) {
    while(*format != '\0') {
        if(*format != '%') {
            format_emit(sink, *format);
            format++;

            continue;
        }

        format++;

        // A '%' at the very end of the format string has nothing to convert.
        if(*format == '\0') {
            break;
        }

        bool left = false;
        bool zero = false;

        while(*format == '-' || *format == '0') {
            if(*format == '-') {
                left = true;
            } else {
                zero = true;
            }

            format++;
        }

        int width = 0;

        while(*format >= '0' && *format <= '9') {
            width = width * 10 + (*format - '0');
            format++;
        }

        // Padding with zeros is meaningless once the value is pushed to the left.
        if(left) {
            zero = false;
        }

        char body[FORMAT_BODY_LENGTH];
        char character[2];

        const char* prefix = "";
        const char* text = NULL;

        switch(*format) {
            case '%': {
                character[0] = '%';
                character[1] = '\0';

                text = character;

                break;
            }
            case 'c': {
                character[0] = (char) va_arg(args, int);
                character[1] = '\0';

                text = character;

                break;
            }
            case 's': {
                text = va_arg(args, const char*);

                break;
            }
            case 'd': {
                itoa(va_arg(args, int), body, 10);

                text = body;

                // Keep a sign in front of the zeros instead of behind them.
                if(zero && body[0] == '-') {
                    prefix = "-";
                    text = body + 1;
                }

                break;
            }
            case 'x': {
                prefix = "0x";

                itoa(va_arg(args, int), body, 16);

                text = body;

                break;
            }
            case 'p': {
                itoa((int32_t) (uintptr_t) va_arg(args, void*), body, 16);

                text = body;

                break;
            }
            case 'b': {
                prefix = "0b";

                itoa(va_arg(args, int), body, 2);

                text = body;

                break;
            }
            case 'f': {
                gcvt(va_arg(args, double), 2, body);

                text = body;

                break;
            }
            default: {
                break;
            }
        }

        if(text != NULL) {
            format_padded(sink, prefix, text, width, left, zero);
        }

        format++;
    }

    return sink->count;
}
