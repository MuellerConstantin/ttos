#include <drivers/video/vga/vga.h>
#include <drivers/video/vga/textmode.h>
#include <device/device.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>

uint16_t *const vga_tm_video_memory = (uint16_t *const) VGA_TM_VIDEO_MEMORY;
uint8_t *const vga_gfx_video_memory = (uint8_t *const) VGA_GFX_VIDEO_MEMORY;

const vga_video_mode_descriptor_t* vga_current_video_mode = NULL;
extern const vga_video_mode_descriptor_t *const VGA_VIDEO_MODE_DESCRIPTOR_TABLE[VGA_NUM_VIDEO_MODES];

static bool vga_probe(void);
static bool vga_tm_probe(void);
static bool vga_gfx_probe(void);
static void vga_init_registers(uint8_t *config);

int32_t vga_init(vga_video_mode_t mode, bool probe) {
    // Check if the video mode is supported
    if(mode != VGA_80x25_16_TEXT) {
        return -1;
    }

    if(probe && !vga_probe()) {
        return -1;
    }

    const vga_video_mode_descriptor_t* descriptor = VGA_VIDEO_MODE_DESCRIPTOR_TABLE[mode];

    if(descriptor == NULL) {
        return -1;
    }

    vga_init_registers(descriptor->config);
    vga_current_video_mode = descriptor;

    if(probe) {
        video_device_t *device = (video_device_t*) kmalloc(sizeof(video_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->info.name = (char*) kmalloc(15);

        if(!device->info.name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        generate_uuid_v4(&device->info.id);
        strcpy(device->info.name, "VGA Controller");
        device->info.type = DEVICE_TYPE_VIDEO;
        device->info.bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->info.bus.data = NULL;

        if(mode == VGA_80x25_16_TEXT) {
            device->driver = (video_driver_t*) kmalloc(sizeof(video_driver_t));

            if(!device->driver) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            device->driver->tm_probe = vga_tm_probe;
            device->driver->gfx_probe = vga_gfx_probe;
            device->driver->tm.clear = vga_tm_clear;
            device->driver->tm.fill = vga_tm_fill;
            device->driver->tm.write = vga_tm_write;
            device->driver->tm.strwrite = vga_tm_strwrite;
            device->driver->tm.putchar = vga_tm_putchar;
            device->driver->tm.putstr = vga_tm_putstr;
            device->driver->tm.scroll = vga_tm_scroll;
            device->driver->tm.move_cursor = vga_tm_move_cursor;
            device->driver->tm.enable_cursor = vga_tm_enable_cursor;
            device->driver->tm.disable_cursor = vga_tm_disable_cursor;
            device->driver->tm.set_color = vga_tm_set_color;

            vga_tm_init();
        }

        device_register(NULL, device);
    }

    return 0;
}

static bool vga_probe(void) {
    outb(VGA_GC_ADDRESS_REGISTER_PORT, 0x0F);
    return inb(VGA_GC_ADDRESS_REGISTER_PORT) == 0x0F;
}

static bool vga_tm_probe(void) {
    return vga_current_video_mode != NULL && vga_current_video_mode->mode == VGA_80x25_16_TEXT;
}

static bool vga_gfx_probe(void) {
    return vga_current_video_mode != NULL && !vga_tm_probe();
}

static void vga_init_registers(uint8_t *config) {
    // Initialize MISC controller
    outb(VGA_MISC_OUTPUT_REGISTER_WRITE_PORT, *(config++));
    
    // Initialize sequencer
    for(uint8_t index = 0; index < VGA_NUM_SEQ_REGISTERS; ++index) {
        outb(VGA_SEQ_ADDRESS_REGISTER_PORT, index);
        outb(VGA_SEQ_DATA_REGISTER_PORT, *(config++));
    }

    // Initialize CRT controller

    // Unlock CRT controller registers
    outb(VGA_CRTC_ADDRESS_REGISTER_PORT, 0x03);
    outb(VGA_CRTC_DATA_REGISTER_PORT, inb(VGA_CRTC_DATA_REGISTER_PORT) | 0x80);
    outb(VGA_CRTC_ADDRESS_REGISTER_PORT, 0x11);
    outb(VGA_CRTC_DATA_REGISTER_PORT, inb(VGA_CRTC_DATA_REGISTER_PORT) & ~0x80);

    // Make sure CRT controller registers remain unlocked
    config[0x03] = config[0x03] | 0x80;
    config[0x11] = config[0x11] & ~0x80;

    // Initialize CRT controller registers
    for(uint8_t index = 0; index < VGA_NUM_CRTC_REGISTERS; ++index) {
        outb(VGA_CRTC_ADDRESS_REGISTER_PORT, index);
        outb(VGA_CRTC_DATA_REGISTER_PORT, *(config++));
    }

    // Initialize graphics controller
    for(uint8_t index = 0; index < VGA_NUM_GC_REGISTERS; ++index) {
        outb(VGA_GC_ADDRESS_REGISTER_PORT, index);
        outb(VGA_GC_DATA_REGISTER_PORT, *(config++));
    }

    // Initialize attribute controller
    for(uint8_t index = 0; index < VGA_NUM_AC_REGISTERS; ++index) {
        inb(VGA_INPUT_STATUS_1_REGISTER_COLOR_READ_PORT);
        outb(VGA_AC_ADDRESS_REGISTER_PORT, index);
        outb(VGA_AC_ATTRIBUTE_ADDRESS_REGISTER, *(config++));
    }

    // Lock 16-color palette and unblank display
    inb(VGA_INPUT_STATUS_1_REGISTER_COLOR_READ_PORT);
    outb(VGA_AC_ADDRESS_REGISTER_PORT, 0x20);
}
