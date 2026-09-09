#include <drivers/video/vga/tm.h>

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

int32_t vga_tm_init() {
    vga_tm_fill(VGA_TM_BLACK);

    // Trade the blink attribute for bright background colors.
    vga_tm_set_blink(false);

    vga_tm_set_palette_color(
        VGA_TM_THEME_COLOR_INDEX,
        VGA_TM_THEME_COLOR_RED,
        VGA_TM_THEME_COLOR_GREEN,
        VGA_TM_THEME_COLOR_BLUE
    );

    vga_tm_enable_cursor(0, 15);
}
