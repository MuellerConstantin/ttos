#include <multiboot/grub.h>
#include <sys/gdt.h>
#include <sys/idt.h>

void kmain(multiboot_header_t* multiboot_header) {
    gdt_init();
    idt_init();

    while(1);
}
