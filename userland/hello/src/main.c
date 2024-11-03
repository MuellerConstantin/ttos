#include <stdio.h>

void _start() {
    const char message[] = "Hello, Kernel!";

    puts(message);

    while (1) {
        __asm__ volatile ("nop");
    }
}
