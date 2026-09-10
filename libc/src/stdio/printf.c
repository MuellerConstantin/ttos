#include <stdio.h>

#include <format.h>

static void printf_write(format_sink_t* sink, char ch) {
    (void) sink;

    putchar(ch);
}

int printf(const char * format, ... ) {
    va_list args;
    va_start(args, format);

    format_sink_t sink = { .write = printf_write, .buffer = NULL, .count = 0 };

    int count = format_write(&sink, format, args);

    va_end(args);

    return count;
}
