#include <proc.h>
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("usage: sleep <milliseconds>\n");
        return 1;
    }

    uint32_t milliseconds = 0;

    for (const char* cursor = argv[1]; *cursor != '\0'; cursor++) {
        if (*cursor < '0' || *cursor > '9') {
            printf("sleep: not a number: %s\n", argv[1]);
            return 1;
        }

        milliseconds = milliseconds * 10 + (*cursor - '0');
    }

    return sleep(milliseconds) == 0 ? 0 : 1;
}
