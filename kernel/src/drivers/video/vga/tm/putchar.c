#include <drivers/video/vga/tm.h>

extern vga_tm_screen_t vga_tm_screen;

void vga_tm_putchar(char ch) {
    switch(ch) {
        case '\n':
            vga_tm_screen.cursor_x = 0;
            vga_tm_screen.cursor_y++;
            break;
        case '\r':
            vga_tm_screen.cursor_x = 0;
            break;
        case '\b':
            if(vga_tm_screen.cursor_x > 0) {
                vga_tm_screen.cursor_x--;
            }

            vga_tm_write(vga_tm_screen.cursor_y * vga_tm_screen.columns + vga_tm_screen.cursor_x, ' ', vga_tm_screen.fgcolor, vga_tm_screen.bgcolor);
            break;
        case '\t':
            vga_tm_screen.cursor_x = (vga_tm_screen.cursor_x + 8) & ~(8 - 1);
            break;
        default:
            vga_tm_write(vga_tm_screen.cursor_y * vga_tm_screen.columns + vga_tm_screen.cursor_x, ch, vga_tm_screen.fgcolor, vga_tm_screen.bgcolor);
            vga_tm_screen.cursor_x++;
            break;
    }

    // Check if end of line has been reached
    if(vga_tm_screen.cursor_x >= vga_tm_screen.columns) {
        vga_tm_screen.cursor_x = 0;
        vga_tm_screen.cursor_y++;
    }

    // Check if end of screen has been reached
    if(vga_tm_screen.cursor_y >= vga_tm_screen.rows) {
        vga_tm_scroll();
    }

    vga_tm_move_cursor(vga_tm_screen.cursor_y * vga_tm_screen.columns + vga_tm_screen.cursor_x);
}

void vga_tm_putstr(const char *str) {
    while(*str) {
        vga_tm_putchar(*str++);
    }
}
