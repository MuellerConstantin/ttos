#include <stdint.h>
#include <stdlib.h>
#include <multiboot.h>
#include <multiboot_util.h>
#include <kernel.h>
#include <memory/pmm.h>
#include <memory/paging.h>
#include <memory/kheap.h>
#include <descriptors/gdt.h>
#include <descriptors/idt.h>
#include <sys/kpanic.h>
#include <sys/isr.h>
#include <drivers/pic/8259.h>
#include <drivers/pit/8253.h>
#include <drivers/video/vga/textmode.h>
#include <drivers/serial/uart/16550.h>
#include <drivers/input/ps2/keyboard.h>

static void init_cpu();
static void init_memory(multiboot_info_t *multiboot_info);
static void init_drivers(multiboot_info_t *multiboot_info);

void kmain(multiboot_info_t *multiboot_info, uint32_t magic) {
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kpanic("IVALID MULTIBOOT MAGIC NUMBER", NULL);
    }

    if(!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP)) {
        kpanic("NO MEMORY MAP PROVIDED BY MULTIBOOT", NULL);
    }

    isr_cli();

    vga_init(VGA_80x25_16_TEXT);

    vga_tm_strwrite(0, "Kernel loading...", VGA_TM_WHITE, VGA_TM_BLACK);

    init_cpu();
    init_memory(multiboot_info);
    init_drivers(multiboot_info);

    vga_tm_strwrite(80, "Kernel loaded!", VGA_TM_WHITE, VGA_TM_BLACK);

    isr_sti();

    while(1);
}

static void init_cpu() {
    gdt_init();
    idt_init();
}

static void init_memory(multiboot_info_t *multiboot_info) {
    const size_t total_memory = multiboot_get_memory_size(multiboot_info);

    // Initialize the physical memory manager
    pmm_init(total_memory);

    // Enable paging
    paging_init();

    // Initialize the kernel heap
    kheap_init();
}

static void init_drivers(multiboot_info_t *multiboot_info) {
    pic_8259_init();
    pit_8253_init(PIT_8253_COUNTER_0, 1000);
    uart_16550_init(UART_16550_COM1, 115200);
    ps2_keyboard_init();
}
