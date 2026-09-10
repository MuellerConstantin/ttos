#include <drivers/video/vga/vga.h>
#include <drivers/video/vga/tm.h>
#include <drivers/video/vga/gfx.h>
#include <device/device.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/types.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

uint16_t *const vga_tm_video_memory = (uint16_t *const) VGA_TM_VIDEO_MEMORY;
uint8_t *const vga_gfx_video_memory = (uint8_t *const) VGA_GFX_VIDEO_MEMORY;

const vga_video_mode_descriptor_t* vga_current_video_mode = NULL;
extern const vga_video_mode_descriptor_t *const VGA_VIDEO_MODE_DESCRIPTOR_TABLE[VGA_NUM_VIDEO_MODES];

static device_t* vga_claim_device(void);
static bool vga_probe(void);
static bool vga_tm_probe(void);
static bool vga_gfx_probe(void);
static void vga_init_registers(uint8_t *config);

int32_t vga_init(vga_video_mode_t mode, bool probe) {
    // Check if the video mode is supported
    if(mode != VGA_80x25_16_TEXT && mode != VGA_640X480X16_GFX && mode != VGA_320X200X256_GFX) {
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
        device_t* device = vga_claim_device();

        if(mode == VGA_80x25_16_TEXT) {
            device->driver.video = (video_driver_t*) kmalloc(sizeof(video_driver_t));

            if(!device->driver.video) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            device->driver.video->tm_probe = vga_tm_probe;
            device->driver.video->gfx_probe = vga_gfx_probe;
            device->driver.video->tm.fill = vga_tm_fill;
            device->driver.video->tm.write = vga_tm_write;
            device->driver.video->tm.strwrite = vga_tm_strwrite;
            device->driver.video->tm.scroll = vga_tm_scroll;
            device->driver.video->tm.move_cursor = vga_tm_move_cursor;
            device->driver.video->tm.enable_cursor = vga_tm_enable_cursor;
            device->driver.video->tm.disable_cursor = vga_tm_disable_cursor;
            device->driver.video->tm.total_rows = vga_tm_total_rows;
            device->driver.video->tm.total_columns = vga_tm_total_columns;

            vga_tm_init();
        } else if (mode == VGA_640X480X16_GFX) {
            device->driver.video = (video_driver_t*) kmalloc(sizeof(video_driver_t));

            if(!device->driver.video) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            device->driver.video->tm_probe = vga_tm_probe;
            device->driver.video->gfx_probe = vga_gfx_probe;
            device->driver.video->gfx.set_pixel = vga_gfx_set_pixel;
            device->driver.video->gfx.fill = vga_gfx_fill;
            device->driver.video->gfx.draw_rect = vga_gfx_draw_rect;
            device->driver.video->gfx.draw_char = vga_gfx_draw_char;
            device->driver.video->gfx.draw_string = vga_gfx_draw_string;
            device->driver.video->gfx.total_width = vga_gfx_total_width;
            device->driver.video->gfx.total_height = vga_gfx_total_height;

            vga_gfx_init();
        } else if (mode == VGA_320X200X256_GFX) {
            device->driver.video = (video_driver_t*) kmalloc(sizeof(video_driver_t));

            if(!device->driver.video) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            device->driver.video->tm_probe = vga_tm_probe;
            device->driver.video->gfx_probe = vga_gfx_probe;
            device->driver.video->gfx.set_pixel = vga_gfx_set_pixel;
            device->driver.video->gfx.fill = vga_gfx_fill;
            device->driver.video->gfx.draw_rect = vga_gfx_draw_rect;
            device->driver.video->gfx.draw_char = vga_gfx_draw_char;
            device->driver.video->gfx.draw_string = vga_gfx_draw_string;
            device->driver.video->gfx.total_width = vga_gfx_total_width;
            device->driver.video->gfx.total_height = vga_gfx_total_height;

            vga_gfx_init();
        }
    }

    return 0;
}

/*
 * Binds the driver to the display controller the PCI scan has already found, so
 * the card shows up once in the device tree instead of twice. A machine that
 * exposes no PCI display controller still needs a console, so in that case the
 * controller is registered as a platform device.
 */
static device_t* vga_claim_device(void) {
    linked_list_t* pci_devices = (linked_list_t*) device_find_all_by_bus_type(DEVICE_BUS_TYPE_PCI);
    device_t* device = NULL;

    /*
     * Only a VGA compatible controller answers on the legacy ports this driver
     * talks to, so a display controller of any other subtype is the wrong
     * device to label. Which controller actually owns those ports is decided by
     * the VGA enable bit of the bridges above it, but walking them to find out
     * is not worth the effort here: picking wrong mislabels the device tree
     * while the output keeps working, because the driver reaches the hardware
     * through those fixed ports either way.
     */
    linked_list_foreach(pci_devices, node) {
        device_t* candidate = (device_t*) node->data;
        pci_device_t* pci_device = (pci_device_t*) candidate->bus.data;

        if(pci_device->type == PCI_TYPE_DISPLAY_CONTROLLER && pci_device->subtype == PCI_SUBTYPE_VGA_CONTROLLER) {
            device = candidate;
            break;
        }
    }

    linked_list_destroy(pci_devices, false);

    char* name = (char*) kmalloc(15);

    if(!name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(name, "VGA Controller");

    // The scan named the device after its identifiers, the driver knows better.
    if(device) {
        kfree(device->name);

        device->name = name;
        device->type = DEVICE_TYPE_VIDEO;

        return device;
    }

    device = (device_t*) kmalloc(sizeof(device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_generate_id(device->id);

    device->name = name;
    device->type = DEVICE_TYPE_VIDEO;
    device->bus.type = DEVICE_BUS_TYPE_PLATFORM;
    device->bus.data = NULL;
    device->driver.raw = NULL;

    device_register(NULL, device);

    return device;
}

static bool vga_probe(void) {
    outb(VGA_GC_ADDRESS_REGISTER_PORT, 0x0F);
    return inb(VGA_GC_ADDRESS_REGISTER_PORT) == 0x0F;
}

static bool vga_tm_probe(void) {
    return vga_current_video_mode != NULL && vga_current_video_mode->mode == VGA_80x25_16_TEXT;
}

static bool vga_gfx_probe(void) {
    return vga_current_video_mode != NULL && vga_current_video_mode->mode == VGA_320X200X256_GFX;
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
