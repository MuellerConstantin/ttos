#include <stdio.h>
#include <fsio.h>

int getchar(void) {
    char ch;
    int32_t return_value = 0;

    do {
        return_value = fsio_read(FSIO_STDIN, &ch, 1);

        if(return_value == -1) {
            return EOF;
        }
    } while(return_value == 0);

    return ch;
}
