void _start() {
    const char message[] = "Hello, Kernel!";

    __asm__ volatile(
        "mov %0, %%ebx\n"
        "mov $0x00, %%eax\n"
        "int $0x80\n"
        :
        : "r"(message)
    );

    while (1) {
        __asm__ volatile ("nop");
    }
}
