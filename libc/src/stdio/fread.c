#include <stdio.h>
#include <fsio.h>

size_t fread(void* buffer, size_t size, size_t count, FILE* stream) {
    if(buffer == NULL || stream == NULL || size == 0 || count == 0) {
        return 0;
    }

    size_t total_bytes = size * count;
    size_t bytes_read = 0;

    while(bytes_read < total_bytes) {
        size_t bytes_to_read = total_bytes - bytes_read;
        int32_t result = fsio_read(stream->fd, (char *) buffer + bytes_read, bytes_to_read);

        if(result <= 0) {
            break;
        }

        bytes_read += result;
    }

    return bytes_read / size;
}
