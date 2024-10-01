#include <drivers/video/vga/gfx.h>

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

vga_gfx_screen_t vga_gfx_screen;

int32_t vga_gfx_init() {
    vga_gfx_screen.width = vga_current_video_mode->width;
    vga_gfx_screen.height = vga_current_video_mode->height;
    vga_gfx_screen.cursor_x = 0;
    vga_gfx_screen.cursor_y = 0;
    vga_gfx_screen.fgcolor = VGA_GFX_WHITE;
    vga_gfx_screen.bgcolor = VGA_GFX_BLACK;

    if(vga_current_video_mode->mode == VGA_640X480X16_GFX) {
        // Enable all planes
        vga_set_write_mode(0x02);
        vga_set_plane_mask(0xF);
    }

    vga_gfx_clear();

    return 0;
}
