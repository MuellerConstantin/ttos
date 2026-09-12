#include <stdint.h>
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
#include <drivers/storage/ramdisk.h>
#include <fs/initfs.h>
#include <drivers/storage/sata.h>
#include <fs/mount.h>
#include <io/tty.h>
#include <io/stream.h>
#include <system/process.h>

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

    /*
     * Map every multiboot module into the kernel's address space. The bootloader
     * placed them in physical memory; they stay there for the life of the
     * system and are handed out as RAM disks later.
     */
    multiboot_module_t* modules = (multiboot_module_t*) (multiboot_info->mods_addr + VMM_KERNEL_SPACE_BASE);

    for(uint32_t index = 0; index < multiboot_info->mods_count; index++) {
        multiboot_module_t* module = &modules[index];
        void* module_start_physical = (void*) module->mod_start;
        size_t module_size = module->mod_end - module->mod_start;

        void* module_start_virtual = vmm_map_memory(NULL, module_size, module_start_physical, true, true);

        /*
         * Both ends have to move, otherwise the module looks like it spans the
         * distance between the two address spaces rather than its own length.
         */
        module->mod_start = (uint32_t) module_start_virtual;
        module->mod_end = (uint32_t) module_start_virtual + module_size;
    }
}

static void init_kernel(multiboot_info_t *multiboot_info) {
    // Initialize the kernel heap
    kheap_init();

    // Initialize the device manager
    device_init();

    // Initialize the volume manager
    volume_init();

    // Initialize the syscall manager
    syscall_init();

    multiboot_info = (multiboot_info_t*) ((uintptr_t) multiboot_info + VMM_KERNEL_SPACE_BASE);

    /*
     * Every multiboot module becomes a RAM disk. The initial ramdisk announces
     * itself with a magic in front of its file system and is kept read-only;
     * anything else is a plain disk in memory that the volume manager probes
     * for a file system like any other.
     */
    multiboot_module_t* modules = (multiboot_module_t*) (multiboot_info->mods_addr + VMM_KERNEL_SPACE_BASE);

    for(uint32_t index = 0; index < multiboot_info->mods_count; index++) {
        void* module_start = (void*) modules[index].mod_start;
        size_t module_size = modules[index].mod_end - modules[index].mod_start;

        if(module_size >= sizeof(uint16_t) && *((uint16_t*) module_start) == INITRD_HEADER_MAGIC) {
            ramdisk_register("Initial Ramdisk", (void*) ((uintptr_t) module_start + sizeof(uint16_t)), module_size - sizeof(uint16_t), true);
        } else {
            ramdisk_register("RAM Disk", module_start, module_size, false);
        }
    }
}

static void init_drivers() {
    /*
     * The bus scan runs first so that the drivers below it can bind to the
     * devices it finds, instead of registering a second device for hardware
     * that is already in the tree or hanging their own devices off the root.
     */
    pci_init();

    vga_init(VGA_80x25_16_TEXT, true);
    kmessage_echo_enable();

    /*
     * The timer only has to be running before interrupts are enabled, which
     * happens after all of this, so it can wait for the bus scan and register
     * itself where it belongs.
     */
    timer_init();

    uart_16550_init(UART_16550_COM1, 115200);
    ps2_keyboard_init();
    ata_init();
    sata_init();
}

static void init_console() {
    // The console takes the screen from here, the log goes back to being a log.
    kmessage_echo_disable();

    // Temporary mount the initial ramdisk to setup the CLI

    volume_t* initrd_volume = volume_find_by_name("Initial Ramdisk");

    if(!initrd_volume) {
        KPANIC(KPANIC_NO_INITRD_DEVICE_FOUND_CODE, KPANIC_NO_INITRD_DEVICE_FOUND_MESSAGE, NULL);
    }

    if(mnt_volume_mount(DRIVE_A, initrd_volume) != 0) {
        KPANIC(KPANIC_INITRD_MOUNT_FAILED_CODE, KPANIC_INITRD_MOUNT_FAILED_MESSAGE, NULL);
    }

    /*
     * Every program the system relies on, init and the shell included, is
     * spawned from the initial ramdisk. Once it is gone, nothing can be
     * started any more, not even the program that would mount it back.
     */
    mnt_drive_lock(DRIVE_A);

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

    // Launch the init process (PID 1). It runs in userland, never exits and is
    // responsible for keeping a shell running. process_run does not return; if
    // init ever exits, process_terminate raises a kernel panic.

    const char* init_path = "A:/init.elf";
    const char* init_argv[] = { init_path };

    // init starts with an empty environment and builds its own; its working directory is the root of the ramdisk.
    process_t* init = process_create("init", init_path, 1, init_argv, 0, NULL, "A:/", out_stream, in_stream, err_stream);

    if(!init) {
        KPANIC(KPANIC_INIT_START_FAILED_CODE, KPANIC_INIT_START_FAILED_MESSAGE, NULL);
    }

    process_run(init);
}
