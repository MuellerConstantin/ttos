#include <drivers/video/vga/gfx.h>

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

int32_t vga_gfx_init() {
    if(vga_current_video_mode->mode == VGA_640X480X16_GFX) {
        // Enable all planes
        vga_set_write_mode(0x02);
        vga_set_plane_mask(0xF);
    }

    vga_gfx_fill(VGA_GFX_BLACK);

    return 0;
}
