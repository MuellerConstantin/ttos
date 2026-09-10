#include <stdio.h>

#include <format.h>

static void sprintf_write(format_sink_t* sink, char ch) {
    // The core has not counted this character yet, so count is the write index.
    sink->buffer[sink->count] = ch;
}

int sprintf(char * str, const char * format, ... ) {
    va_list args;
    va_start(args, format);

    format_sink_t sink = { .write = sprintf_write, .buffer = str, .count = 0 };

    int count = format_write(&sink, format, args);

    va_end(args);

    str[count] = '\0';

    return count;
}
