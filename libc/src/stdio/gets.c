#include <stdio.h>

char *gets(char *str) {
    char ch;
    int index = 0;

    while ((ch = getchar()) != '\n') {
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

    str[index] = '\0';

    return str;
}
