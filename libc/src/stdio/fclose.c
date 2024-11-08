#include <stdio.h>
#include <fsio.h>

int fclose(FILE* stream) {
    if(stream == NULL) {
        return EOF;
    }

    int32_t result = fsio_close(stream->fd);

    if(result < 0) {
        return EOF;
    }

    return 0;
}
