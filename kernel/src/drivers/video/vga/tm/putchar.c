#include <drivers/video/vga/textmode.h>

extern vga_tm_screen_t vga_tm_screen;

void vga_tm_putchar(char ch, uint8_t fgcolor, uint8_t bgcolor) {
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

            vga_tm_write(vga_tm_screen.cursor_y * vga_tm_screen.columns + vga_tm_screen.cursor_x, ' ', fgcolor, bgcolor);
            break;
        default:
            vga_tm_write(vga_tm_screen.cursor_y * vga_tm_screen.columns + vga_tm_screen.cursor_x, ch, fgcolor, bgcolor);
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

void vga_tm_putstr(const char *str, uint8_t fgcolor, uint8_t bgcolor) {
    while(*str) {
        vga_tm_putchar(*str++, fgcolor, bgcolor);
    }
}
