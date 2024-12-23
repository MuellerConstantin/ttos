#include <stdint.h>
#include <stdlib.h>
#include <multiboot.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <memory/kheap.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/tss.h>
#include <arch/i386/isr.h>
#include <arch/i386/acpi.h>
#include <arch/i386/pic/8259.h>
#include <system/timer.h>
#include <system/kpanic.h>
#include <system/kmessage.h>
#include <system/syscall.h>
#include <device/device.h>
#include <device/volume.h>
#include <drivers/pci/pci.h>
#include <drivers/video/vga/vga.h>
#include <drivers/serial/uart/16550.h>
#include <drivers/input/ps2/keyboard.h>
#include <drivers/storage/ata.h>
#include <drivers/storage/initrd.h>
#include <drivers/storage/sata.h>
#include <fs/mount.h>
#include <io/tty.h>
#include <io/stream.h>
#include <shell/shell.h>

static void init_platform(multiboot_info_t *multiboot_info);
static void init_kernel(multiboot_info_t *multiboot_info);
static void init_drivers();
static void init_console();

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

    kmessage_init();

    kmessage(KMESSAGE_LEVEL_INFO, "Booting kernel...");
    kmessage(KMESSAGE_LEVEL_INFO, "multiboot: Booted by a Multiboot-compliant bootloader");
    kmessage(KMESSAGE_LEVEL_INFO, "multiboot: Memory map provided by bootloader");
    kmessage(KMESSAGE_LEVEL_INFO, "multiboot: Modules provided by bootloader");

    init_platform(multiboot_info);
    init_kernel(multiboot_info);
    init_drivers();

    isr_sti();

    // init_usermode();

    init_console();

    while(1);
}

static void init_platform(multiboot_info_t *multiboot_info) {
    gdt_init();
    idt_init();
    tss_init(0x10, 0x0);
    pmm_init(multiboot_info);
    vmm_init();
    acpi_init();
    pic_8259_init();

    /*
     * Save the kernel stack pointer to the TSS so that the kernel has a valid
     * stack pointer when switching back to kernel mode in case of an exception.
     */

    uint32_t kernel_stack;
    __asm__ volatile("mov %%esp, %0" : "=r" (kernel_stack));

    tss_update_ring0_stack(0x10, kernel_stack);

    multiboot_info = (multiboot_info_t*) ((uintptr_t) multiboot_info + VMM_KERNEL_SPACE_BASE);

    // Map the initial ramdisk multiboot module to virtual address space
    if(multiboot_info->mods_count > 0) {
        multiboot_module_t *initrd_module = multiboot_info->mods_addr + VMM_KERNEL_SPACE_BASE;
        void* initrd_start_physical = (void*) initrd_module->mod_start;
        size_t initrd_size = initrd_module->mod_end - initrd_module->mod_start;

        // Map the initrd's virtual address space
        void* initrd_start_virtual = vmm_map_memory(NULL, initrd_size, initrd_start_physical, true, true);

        initrd_module->mod_start = (uint32_t) initrd_start_virtual;
    }
}

static void init_kernel(multiboot_info_t *multiboot_info) {
    // Initialize the kernel heap
    kheap_init();

    // Initialize the device manager
    device_init();

    // Initialize the volume manager
    volume_init();

    // Initialize the system timer
    timer_init();

    // Initialize the syscall manager
    syscall_init();

    multiboot_info = (multiboot_info_t*) ((uintptr_t) multiboot_info + VMM_KERNEL_SPACE_BASE);

    // Load initial ramdisk multiboot module if provided
    if(multiboot_info->mods_count > 0) {
        multiboot_module_t *initrd_module = multiboot_info->mods_addr + VMM_KERNEL_SPACE_BASE;
        void* initrd_start = (void*) initrd_module->mod_start;
        size_t initrd_size = initrd_module->mod_end - initrd_module->mod_start;

        initrd_init((void*) initrd_start, initrd_size);
    }
}

static void init_drivers() {
    uart_16550_init(UART_16550_COM1, 115200);
    vga_init(VGA_80x25_16_TEXT, true);
    ps2_keyboard_init();
    pci_init();
    ata_init();
    sata_init();
}

static void init_console() {
    // Temporary mount the initial ramdisk to setup the CLI

    volume_t* initrd_volume = volume_find_by_name("Initial Ramdisk");

    if(!initrd_volume) {
        KPANIC(KPANIC_NO_INITRD_DEVICE_FOUND_CODE, KPANIC_NO_INITRD_DEVICE_FOUND_MESSAGE, NULL);
    }

    if(mnt_volume_mount(DRIVE_A, initrd_volume) != 0) {
        KPANIC(KPANIC_INITRD_MOUNT_FAILED_CODE, KPANIC_INITRD_MOUNT_FAILED_MESSAGE, NULL);
    }

    // Ensure that io devices required for the CLI are available

    video_device_t* video_device = device_find_by_type(DEVICE_TYPE_VIDEO);
    keyboard_device_t* keyboard_device = device_find_by_type(DEVICE_TYPE_KEYBOARD);

    if(!video_device) {
        KPANIC(KPANIC_NO_OUTPUT_DEVICE_FOUND_CODE, KPANIC_NO_OUTPUT_DEVICE_FOUND_MESSAGE, NULL);
    }

    if(!keyboard_device) {
        KPANIC(KPANIC_NO_INPUT_DEVICE_FOUND_CODE, KPANIC_NO_INPUT_DEVICE_FOUND_MESSAGE, NULL);
    }

    tty_t* tty0 = tty_create(video_device, keyboard_device, &tty_keyboard_layout_de_DE);
    tty_set_stdterm(tty0);

    stream_t* out_stream = tty_get_out_stream(tty0);
    stream_t* in_stream = tty_get_in_stream(tty0);
    stream_t* err_stream = tty_get_err_stream(tty0);

    // Initialize the CLI

    shell_init(out_stream, in_stream, err_stream);
    shell_execute();
}
