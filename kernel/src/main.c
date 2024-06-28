#include <stdint.h>
#include <multiboot.h>
#include <descriptors/gdt.h>
#include <descriptors/idt.h>
#include <sys/isr.h>
#include <drivers/pic/8259.h>
#include <drivers/pit/8253.h>
#include <drivers/video/vga/textmode.h>

void kmain(multiboot_info_t *multiboot_info, uint32_t magic) {
    vga_init(VGA_80x25_16_TEXT);

    vga_tm_strwrite(0, "Kernel loading...", VGA_TM_WHITE, VGA_TM_BLACK);

    isr_cli();

    gdt_init();
    idt_init();
    pic_8259_init();
    pit_8253_init(PIT_8253_COUNTER_0, 1000);

    isr_sti();

    vga_tm_strwrite(80, "Kernel loaded!", VGA_TM_WHITE, VGA_TM_BLACK);

    while(1);
}
