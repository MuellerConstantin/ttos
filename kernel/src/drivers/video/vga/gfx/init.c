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

    return 0;
}
