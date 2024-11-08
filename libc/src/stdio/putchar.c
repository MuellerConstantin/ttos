#include <stdio.h>
#include <fsio.h>

int putchar(int ch) {
    int32_t return_value = fsio_write(FSIO_STDOUT, &ch, 1);

    if(return_value < 0) {
        return EOF;
    }

    return ch;
}
