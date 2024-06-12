#include <multiboot/grub.h>
#include <sys/gdt.h>

void kmain(multiboot_header_t* multiboot_header) {
    gdt_init();

    while(1);
}
