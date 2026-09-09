#include <drivers/video/vga/tm.h>

void vga_tm_set_blink(bool enabled) {
    /*
     * The attribute controller multiplexes address and data on one port and
     * alternates between them on every write. Reading the input status register
     * resets that flip-flop, so the next write is taken as the address.
     */
    inb(VGA_INPUT_STATUS_1_REGISTER_COLOR_READ_PORT);

    /*
     * Selecting a register with the palette address source cleared blanks the
     * screen, so the bit has to stay set while addressing.
     */
    outb(VGA_AC_ADDRESS_REGISTER_PORT, VGA_AC_MODE_CONTROL_REGISTER | VGA_AC_PALETTE_ADDRESS_SOURCE);

    uint8_t mode = inb(VGA_AC_READ_REGISTER_PORT);

    if(enabled) {
        mode |= VGA_AC_MODE_CONTROL_BLINK_ENABLE;
    } else {
        mode &= ~VGA_AC_MODE_CONTROL_BLINK_ENABLE;
    }

    /*
     * The flip-flop now points at the data half, so this write updates the
     * register selected above.
     */
    outb(VGA_AC_ADDRESS_REGISTER_PORT, mode);
}
