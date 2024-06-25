#include <stdint.h>
#include <multiboot.h>
#include <descriptors/gdt.h>
#include <descriptors/idt.h>
#include <sys/isr.h>
#include <drivers/pic/8259.h>
#include <drivers/serial/uart/16550.h>

void kmain(multiboot_info_t *multiboot_info, uint32_t magic) {

    uart_16550_init(UART_16550_COM1, 115200);

    uart_16550_write(UART_16550_COM1, "Kernel loading...\n", 18);

    isr_cli();

    gdt_init();
    idt_init();
    pic_8259_init();

    isr_sti();

    uart_16550_write(UART_16550_COM1, "Kernel loaded!\n", 15);

    while(1);
}
