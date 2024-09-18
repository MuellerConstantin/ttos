#include <drivers/video/vga/textmode.h>

extern vga_tm_screen_t vga_tm_screen;

void vga_tm_set_color(uint8_t fgcolor, uint8_t bgcolor) {
    vga_tm_screen.fgcolor = fgcolor;
    vga_tm_screen.bgcolor = bgcolor;
}
