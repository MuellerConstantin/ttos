#include <fsio.h>
#include <stdio.h>
#include <sysinfo.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        puts("usage: more <path>\n");
        return 1;
    }

    // Query the terminal size, falling back to a sane default if the syscall fails.
    terminfo_t term;
    uint32_t rows = 25;
    uint32_t cols = 80;

    if (sysinfo_get_terminfo(&term) == 0 && term.rows > 1 && term.cols > 0) {
        rows = term.rows;
        cols = term.cols;
    }

    int32_t fd = fsio_open(argv[1], FSIO_RDONLY, 0);

    if (fd < 0) {
        puts("more: cannot open file\n");
        return 1;
    }

    char buffer[512];
    int32_t bytes_read;

    uint32_t used_rows = 0;      // screen rows filled on the current page
    uint32_t used_cols = 0;      // columns filled on the current line
    int quit = 0;

    while (!quit && (bytes_read = fsio_read(fd, buffer, sizeof(buffer))) > 0) {
        for (int32_t i = 0; i < bytes_read && !quit; i++) {
            char ch = buffer[i];

            putchar(ch);

            // Track cursor movement, accounting for lines that wrap past the width.
            if (ch == '\n') {
                used_rows++;
                used_cols = 0;
            } else {
                used_cols++;

                if (used_cols >= cols) {
                    used_rows++;
                    used_cols = 0;
                }
            }

            // One row is reserved for the prompt, so pause after rows - 1 filled rows.
            if (used_rows >= rows - 1) {
                putchar(':');

                // Wait for a key: 'q' quits, 'n' or Enter shows the next page.
                int c;

                do {
                    c = getchar();

                    if (c == 'q') {
                        quit = 1;
                    }
                } while (!quit && c != 'n' && c != '\n');

                // Erase the prompt character.
                putchar('\b');
                putchar(' ');
                putchar('\b');

                used_rows = 0;
                used_cols = 0;
            }
        }
    }

    fsio_close(fd);

    return 0;
}
