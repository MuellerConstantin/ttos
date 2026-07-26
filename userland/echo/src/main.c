#include <stdio.h>

int main(int argc, char** argv) {
    // argv[0] is the program path, so the echoed arguments start at index 1.
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);

        if (i < argc - 1) {
            printf(" ");
        }
    }

    printf("\n");

    return 0;
}
