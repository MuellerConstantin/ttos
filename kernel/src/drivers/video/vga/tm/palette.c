#include <drivers/video/vga/tm.h>

/*
 * Rewrites one entry of the DAC color table. Components are 6 bit, so 0 to 63
 * rather than 0 to 255. Text mode reaches the DAC through the attribute
 * controller's palette registers, which mode 3 sets up as an identity mapping
 * for the first six colors, so entry N is what attribute color N renders as.
 */
void vga_tm_set_palette_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
    outb(VGA_DAC_ADDRESS_WRITE_MODE_REGISTER, index);

    // The DAC auto-advances through the three components on consecutive writes.
    outb(VGA_DAC_DATA_REGISTER, red);
    outb(VGA_DAC_DATA_REGISTER, green);
    outb(VGA_DAC_DATA_REGISTER, blue);
}
