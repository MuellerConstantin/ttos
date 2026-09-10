#include <stdio.h>
#include <string.h>
#include <fsio.h>

int puts(const char* str) {
    /*
     * Deviating from the standard, no newline is appended. Callers pass one in
     * the string when they want it.
     */

    int32_t return_value = fsio_write(FSIO_STDOUT, str, strlen(str));

    if(return_value < 0) {
        return EOF;
    }

    return return_value;
}
