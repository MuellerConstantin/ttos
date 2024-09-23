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
#include <device/device.h>
#include <drivers/pci/pci.h>
#include <drivers/pic/8259.h>
#include <drivers/pit/8253.h>
#include <drivers/video/vga/vga.h>
#include <drivers/serial/uart/16550.h>
#include <drivers/input/ps2/keyboard.h>
#include <drivers/storage/ata.h>
#include <fs/mount.h>
#include <fs/initrd.h>
#include <io/tty.h>
#include <io/shell.h>
#include <sys/switch_usermode.h>

static void init_cpu();
static void init_memory(multiboot_info_t *multiboot_info);
static void init_drivers(multiboot_info_t *multiboot_info);
static void init_filesystem(multiboot_info_t *multiboot_info);
static void init_usermode();

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

    // init_usermode();

    device_t* video_device = device_find_by_type(DEVICE_TYPE_VIDEO);
    device_t* keyboard_device = device_find_by_type(DEVICE_TYPE_KEYBOARD);

    if(!video_device) {
        KPANIC(KPANIC_DEVICE_NO_OUTPUT_DEVICE_FOUND_CODE, KPANIC_DEVICE_NO_OUTPUT_DEVICE_FOUND_MESSAGE, NULL);
    }

    if(!keyboard_device) {
        KPANIC(KPANIC_DEVICE_NO_INPUT_DEVICE_FOUND_CODE, KPANIC_DEVICE_NO_INPUT_DEVICE_FOUND_MESSAGE, NULL);
    }

    tty_t* tty0 = tty_create(video_device->driver.video, keyboard_device->driver.keyboard, &tty_keyboard_layout_de_DE);

    tty_printf(tty0, " _____  _____  ____  ____ \n");
    tty_printf(tty0, "/__ __\\/__ __\\/  _ \\/ ___\\\n");
    tty_printf(tty0, "  / \\    / \\  | / \\||    \\\n");
    tty_printf(tty0, "  | |    | |  | \\_/|\\___ |\n");
    tty_printf(tty0, "  \\_/    \\_/  \\____/\\____/\n");
    tty_printf(tty0, "Tiny Toy Operating System\n");
    tty_printf(tty0, "-*-*-*-*-*-*-*-*-*-*-*-*-*\n\n");

    shell_t* shell = shell_create(tty0);
    shell_execute(shell);

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
    vga_init(VGA_80x25_16_TEXT, true);
    pit_8253_init(PIT_8253_COUNTER_0, 1000);
    uart_16550_init(UART_16550_COM1, 115200);
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

static void init_usermode() {
    /*
     * Save the kernel stack pointer to the TSS so that the kernel has a valid
     * stack pointer when switching back to kernel mode in case of an exception.
     */

    uint32_t kernel_stack;
    __asm__ volatile("mov %%esp, %0" : "=r" (kernel_stack));

    tss_update_ring0_stack(0x10, kernel_stack);

    // TODO: Launch a user process with separate address space

    switch_usermode();
}
