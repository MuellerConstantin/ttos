#include <stdint.h>
#include <stdlib.h>
#include <multiboot.h>
#include <multiboot_util.h>
#include <kernel.h>
#include <memory/pmm.h>
#include <memory/paging.h>
#include <descriptors/gdt.h>
#include <descriptors/idt.h>
#include <sys/kpanic.h>
#include <sys/isr.h>
#include <drivers/pic/8259.h>
#include <drivers/pit/8253.h>
#include <drivers/video/vga/textmode.h>
#include <drivers/serial/uart/16550.h>

static void init_cpu();
static void init_memory(size_t total_memory);
static void init_drivers();

void kmain(multiboot_info_t *multiboot_info, uint32_t magic) {
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kpanic("IVALID MULTIBOOT MAGIC NUMBER", NULL);
    }

    if(!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP)) {
        kpanic("NO MEMORY MAP PROVIDED BY MULTIBOOT", NULL);
    }

    const size_t total_memory = multiboot_get_memory_size(multiboot_info);

    isr_cli();

    vga_init(VGA_80x25_16_TEXT);

    vga_tm_strwrite(0, "Kernel loading...", VGA_TM_WHITE, VGA_TM_BLACK);

    init_cpu();
    init_memory(total_memory);
    init_drivers();

    vga_tm_strwrite(80, "Kernel loaded!", VGA_TM_WHITE, VGA_TM_BLACK);

    isr_sti();

    while(1);
}

static void init_cpu() {
    gdt_init();
    idt_init();
}

static void init_memory(size_t total_memory) {
    // Initialize the physical memory manager
    pmm_init(total_memory);

    // Mark the VGA video memory as reserved
    pmm_mark_region_reserved((void*) 0x000A0000, 0x60000);

    // Mark the 4MB of kernel space and kernel heap placement memory as reserved
    pmm_mark_region_reserved((void*) &kernel_physical_start, 0x400000);

    // Enable paging
    paging_init();
}

static void init_drivers() {
    pic_8259_init();
    pit_8253_init(PIT_8253_COUNTER_0, 1000);
    uart_16550_init(UART_16550_COM1, 115200);
}
