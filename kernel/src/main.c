#include <stdint.h>
#include <stdlib.h>
#include <multiboot.h>
#include <multiboot_util.h>
#include <memory/pmm.h>
#include <descriptors/gdt.h>
#include <descriptors/idt.h>
#include <sys/panic.h>
#include <sys/isr.h>
#include <drivers/pic/8259.h>
#include <drivers/pit/8253.h>
#include <drivers/video/vga/textmode.h>

extern uint32_t kernel_start;
extern uint32_t kernel_end;

static void init_cpu();
static void init_memory(multiboot_info_t *multiboot_info);
static void init_drivers();

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
    init_drivers();

    vga_tm_strwrite(80, "Kernel loaded!", VGA_TM_WHITE, VGA_TM_BLACK);

    isr_sti();

    while(1);
}

static void init_cpu() {
    gdt_init();
    idt_init();
    pic_8259_init();
}

static void init_memory(multiboot_info_t *multiboot_info) {
    const size_t total_memory = multiboot_get_memory_size(multiboot_info);
    pmm_init(total_memory, (uint32_t) &kernel_end);

    // Reserve used memory regions based on the multiboot memory map
    for(uint32_t offset = 0;
        offset < multiboot_info->mmap_length;
        offset += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t *mmap = (multiboot_memory_map_t *) (multiboot_info->mmap_addr + offset);

        if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            pmm_mark_region_available(mmap->addr, mmap->len);
        }
    }

    // Reserve the first 1MB of memory
    pmm_mark_region_reserved(0, 0x100000);

    // Reserve the kernel memory
    pmm_mark_region_reserved((uint32_t) &kernel_start, (uint32_t) &kernel_end - (uint32_t) &kernel_start);

    // Reserve memory for memory management structures
    pmm_mark_region_reserved((uint32_t) &kernel_end, 0x1000);

    // Reserve memory for the VGA video memory and BIOS data area
    pmm_mark_region_reserved(0x000A0000, 0x60000);
}

static void init_drivers() {
    pit_8253_init(PIT_8253_COUNTER_0, 1000);
}
