#include <stdio.h>

char *gets(char *str) {
    int ch;
    int index = 0;

    // Input may end early: a process that is not allowed to read gets EOF.
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if(ch == '\b') {
            if(index == 0) {
                continue;
            }

            index--;
            putchar(ch);

            continue;
        }

        putchar(ch);

        str[index] = ch;
        index++;
    }

    // Echo the newline that terminated the line, mirroring canonical-mode input.
    putchar('\n');

    str[index] = '\0';

    return str;
}
