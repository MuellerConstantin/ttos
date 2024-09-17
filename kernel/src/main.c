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
#include <descriptors/tss.h>
#include <sys/kpanic.h>
#include <sys/isr.h>
#include <drivers/device.h>
#include <drivers/pci/pci.h>
#include <drivers/pic/8259.h>
#include <drivers/pit/8253.h>
#include <drivers/video/vga/textmode.h>
#include <drivers/serial/uart/16550.h>
#include <drivers/input/keyboard.h>
#include <drivers/input/ps2/keyboard.h>
#include <drivers/storage/ata.h>
#include <fs/mount.h>
#include <fs/initrd.h>

static void init_cpu();
static void init_memory(multiboot_info_t *multiboot_info);
static void init_drivers(multiboot_info_t *multiboot_info);
static void init_filesystem(multiboot_info_t *multiboot_info);

void kmain(multiboot_info_t *multiboot_info, uint32_t magic) {
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        KPANIC(KPANIC_INVALID_MULTIBOOT_SIGNATURE_CODE, KPANIC_INVALID_MULTIBOOT_SIGNATURE_MESSAGE, NULL);
    }

    if(!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP)) {
        KPANIC(KPANIC_NO_MEMORY_MAP_CODE, KPANIC_NO_MEMORY_MAP_MESSAGE, NULL);
    }

    if(!(multiboot_info->flags & MULTIBOOT_INFO_MODS || multiboot_info->mods_count == 0)) {
        KPANIC(KPANIC_NO_MODULES_PROVIDED_CODE, KPANIC_NO_MODULES_PROVIDED_MESSAGE, NULL);
    }

    isr_cli();

    init_cpu();
    init_memory(multiboot_info);
    init_drivers(multiboot_info);
    init_filesystem(multiboot_info);

    isr_sti();

    vga_tm_putstr(" _____  _____  ____  ____ \n", VGA_TM_WHITE, VGA_TM_BLACK);
    vga_tm_putstr("/__ __\\/__ __\\/  _ \\/ ___\\\n", VGA_TM_WHITE, VGA_TM_BLACK);
    vga_tm_putstr("  / \\    / \\  | / \\||    \\\n", VGA_TM_WHITE, VGA_TM_BLACK);
    vga_tm_putstr("  | |    | |  | \\_/|\\___ |\n", VGA_TM_WHITE, VGA_TM_BLACK);
    vga_tm_putstr("  \\_/    \\_/  \\____/\\____/\n", VGA_TM_WHITE, VGA_TM_BLACK);
    vga_tm_putstr("Tiny Toy Operating System\n", VGA_TM_WHITE, VGA_TM_BLACK);
    vga_tm_putstr("-*-*-*-*-*-*-*-*-*-*-*-*-*\n", VGA_TM_WHITE, VGA_TM_BLACK);

    while(1);
}

static void init_cpu() {
    gdt_init();
    idt_init();
    tss_init(0x10, 0x0);
}

static void init_memory(multiboot_info_t *multiboot_info) {
    const size_t total_memory = multiboot_get_memory_size(multiboot_info);

    if(total_memory < KERNEL_MINIMAL_RAM_SIZE) {
        KPANIC(KPANIC_RAM_MINIMAL_SIZE_CODE, KPANIC_RAM_MINIMAL_SIZE_MESSAGE, NULL);
    }

    // Initialize the physical memory manager
    pmm_init(total_memory);

    // Enable paging
    paging_init();

    // Map the initrd's virtual address space
    multiboot_info = (multiboot_info_t*) ((uintptr_t) multiboot_info + KERNEL_SPACE_BASE);
    multiboot_module_t *initrd_module = multiboot_info->mods_addr + KERNEL_SPACE_BASE;
    uint32_t initrd_start = (uint32_t) initrd_module->mod_start + KERNEL_SPACE_BASE;
    size_t initrd_size = initrd_module->mod_end - initrd_module->mod_start;

    paging_map_memory((void*) initrd_start, initrd_size, (void*) initrd_start - KERNEL_SPACE_BASE, true, true);

    // Initialize the kernel heap
    kheap_init();
}

static void init_drivers(multiboot_info_t *multiboot_info) {
    device_init();
    pic_8259_init();
    vga_tm_init(true);
    pit_8253_init(PIT_8253_COUNTER_0, 1000);
    uart_16550_init(UART_16550_COM1, 115200);
    keyboard_init();
    ps2_keyboard_init();
    pci_init();
    ata_init();
}

static void init_filesystem(multiboot_info_t *multiboot_info) {
    multiboot_info = (multiboot_info_t*) ((uintptr_t) multiboot_info + KERNEL_SPACE_BASE);
    multiboot_module_t *initrd_module = multiboot_info->mods_addr + KERNEL_SPACE_BASE;
    uint32_t initrd_start = (uint32_t) initrd_module->mod_start + KERNEL_SPACE_BASE;

    mnt_volume_t* initrd_volume = initrd_init((void*) initrd_start);

    if(!initrd_volume) {
        KPANIC(KPANIC_INITRD_INIT_FAILED_CODE, KPANIC_INITRD_INIT_FAILED_MESSAGE, NULL);
    }

    mnt_volume_mount(DRIVE_A, initrd_volume);
}
