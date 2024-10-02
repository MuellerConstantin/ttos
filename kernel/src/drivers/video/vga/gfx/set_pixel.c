#include <drivers/video/vga/gfx.h>
#include <drivers/video/vga/vga.h>

extern uint8_t *const vga_gfx_video_memory;

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

static int32_t vga_gfx_12h_set_pixel(uint32_t x, uint32_t y, uint32_t color);

int32_t vga_gfx_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if(x >= vga_current_video_mode->width || y >= vga_current_video_mode->height) {
        return -1;
    }

    if(vga_current_video_mode->mode == VGA_640X480X16_GFX) {
        return vga_gfx_12h_set_pixel(x, y, color);
    }

    vga_gfx_video_memory[y * vga_current_video_mode->width + x] = color;

    return 0;
}

static int32_t vga_gfx_12h_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if(x >= vga_current_video_mode->width || y >= vga_current_video_mode->height) {
        return -1;
    }

    // Pitch is the number of bytes per row
    uint8_t pitch = vga_current_video_mode->width / 8;

    uint8_t* destination = vga_gfx_video_memory + (y * pitch) + (x >> 3);

    // Read to load current byte across all bitplanes into the latches
    volatile uint8_t tmp = *destination;

    // Set the bit mask
    vga_set_bit_mask(0x80 >> (x & 0x07));

    // Write the pixel
    *destination = color;

    return 0;
}
