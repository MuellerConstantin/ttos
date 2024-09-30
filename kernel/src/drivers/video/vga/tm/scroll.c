#include <drivers/video/vga/tm.h>

extern uint16_t *const vga_tm_video_memory;

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

extern vga_tm_screen_t vga_tm_screen;

void vga_tm_scroll() {
    for (uint16_t index = vga_tm_screen.columns; index < (vga_tm_screen.columns * vga_tm_screen.rows); index++) {
        vga_tm_video_memory[index - 80] = vga_tm_video_memory[index];
    }

    for (uint16_t index = vga_tm_screen.columns * vga_tm_screen.rows; index < (vga_tm_screen.columns * (vga_tm_screen.rows + 1)); index++) {
        vga_tm_video_memory[index - 80] = VGA_TM_ENTRY(' ', vga_tm_screen.fgcolor, vga_tm_screen.bgcolor);
    }

    vga_tm_screen.cursor_y--;

    vga_tm_move_cursor(vga_tm_screen.cursor_y * vga_tm_screen.columns + vga_tm_screen.cursor_x);
}
