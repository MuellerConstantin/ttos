#include <drivers/video/vga/vga.h>

int32_t vga_set_write_mode(uint8_t mode) {
    outb(VGA_GC_ADDRESS_REGISTER_PORT, VGA_GC_MODE_REGISTER);
    outb(VGA_GC_DATA_REGISTER_PORT, mode);

    return 0;
}

int32_t vga_set_plane_mask(uint8_t mask) {
    outb(VGA_SEQ_ADDRESS_REGISTER_PORT, VGA_SEQ_MAP_MASK_REGISTER);
    outb(VGA_SEQ_DATA_REGISTER_PORT, mask);

    return 0;
}

int32_t vga_set_bit_mask(uint8_t mask) {
    outb(VGA_GC_ADDRESS_REGISTER_PORT, VGA_GC_BIT_MASK_REGISTER);
    outb(VGA_GC_DATA_REGISTER_PORT, mask);

    return 0;
}
