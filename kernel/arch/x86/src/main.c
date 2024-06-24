#include <multiboot/grub.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/interrupts.h>
#include <drivers/pic/8259.h>
#include <drivers/uart/16550.h>

void kmain(multiboot_header_t* multiboot_header) {
    uart_16550_init(UART_16550_COM1, 115200);

    uart_16550_write(UART_16550_COM1, "Kernel loading...\n", 18);

    interrupt_disable();

    gdt_init();
    idt_init();
    pic_8259_init();

    interrupt_enable();

    uart_16550_write(UART_16550_COM1, "Kernel loaded!\n", 15);

    while(1);
}
