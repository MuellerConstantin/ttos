#include <stdio.h>

int main(void) {
    // Clear the entire screen and move the cursor to the top-left corner using
    // ANSI/VT100 escape sequences interpreted by the terminal.
    printf("\033[2J\033[H");

    return 0;
}
