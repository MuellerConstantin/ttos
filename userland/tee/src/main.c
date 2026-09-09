#include <fsio.h>
#include <stdio.h>

#define TEE_EOT             0x04
#define TEE_BUFFER_SIZE     512

/*
 * Write the pending bytes to the file. Returns 0 on success or -1 on a short
 * write, which is what a full file system looks like from here.
 */
static int32_t tee_flush(int32_t fd, char* buffer, size_t length) {
    if(length == 0) {
        return 0;
    }

    return (fsio_write(fd, buffer, length) == (int32_t) length) ? 0 : -1;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        puts("usage: tee <path>\n");
        return 1;
    }

    int32_t fd = fsio_open(argv[1], FSIO_WRONLY | FSIO_CREAT | FSIO_TRUNC, 0644);

    if(fd < 0) {
        puts("tee: cannot open file\n");
        return 1;
    }

    char buffer[TEE_BUFFER_SIZE];
    size_t length = 0;
    int32_t status = 0;

    /*
     * Input is read character wise, because the TTY has no canonical mode: it
     * hands over every keystroke as it arrives and echoes nothing. Copying the
     * character to stdout is what tee does anyway, so the input stays visible
     * without any extra work, but Backspace has to be resolved here rather than
     * by a line discipline.
     */
    for(;;) {
        int ch = getchar();

        if(ch == EOF || ch == TEE_EOT) {
            break;
        }

        if(ch == '\b') {
            /*
             * Only the bytes still sitting in the buffer can be taken back;
             * everything before the last flush has already reached the file.
             */
            if(length > 0) {
                length--;
                puts("\b \b");
            }

            continue;
        }

        putchar(ch);

        buffer[length++] = (char) ch;

        // Flush line by line, and whenever the buffer runs full in between.
        if(ch == '\n' || length == TEE_BUFFER_SIZE) {
            if(tee_flush(fd, buffer, length) != 0) {
                puts("tee: write failed\n");
                status = 1;
                break;
            }

            length = 0;
        }
    }

    if(status == 0 && tee_flush(fd, buffer, length) != 0) {
        puts("tee: write failed\n");
        status = 1;
    }

    putchar('\n');

    fsio_close(fd);

    return status;
}
