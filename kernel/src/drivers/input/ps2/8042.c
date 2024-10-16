#include <drivers/input/ps2/8042.h>
#include <system/ports.h>

bool ps2_8042_first_port_probe() {
    // Send controller self-test command
    outb(PS2_COMMAND_REGISTER, 0xAA);

    if(inb(PS2_DATA_REGISTER) != 0x55) {
        return false;
    }

    // Send port self-test command
    outb(PS2_COMMAND_REGISTER, 0xAB);

    return inb(PS2_DATA_REGISTER) == 0x00;
}

void ps2_8042_enable_first_port() {
    outb(PS2_COMMAND_REGISTER, 0xAE);
}

void ps2_8042_disable_first_port(void) {
    outb(PS2_COMMAND_REGISTER, 0xAD);
}

void ps2_8042_init_first_port(bool enable_translation) {
    // PS/2 read command byte
    outb(PS2_COMMAND_REGISTER, 0x20);
    uint8_t command_byte = inb(PS2_DATA_REGISTER);

    // Enable interrupts for first PS/2 port
    command_byte |= 0x01;

    // Enable translation for first PS/2 port (scancode set 1)
    if (enable_translation) {
        command_byte |= 0x40;
    } else {
        command_byte &= ~0x40;
    }

    // Write command byte
    outb(PS2_COMMAND_REGISTER, 0x60);
    outb(PS2_DATA_REGISTER, command_byte);
}

bool ps2_8042_second_port_probe() {
    // Send controller self-test command
    outb(PS2_COMMAND_REGISTER, 0xA9);

    if(inb(PS2_DATA_REGISTER) != 0x00) {
        return false;
    }

    // Send controller self-test command
    outb(PS2_COMMAND_REGISTER, 0xA9);

    return inb(PS2_DATA_REGISTER) == 0x00;
}

void ps2_8042_enable_second_port() {
    outb(PS2_COMMAND_REGISTER, 0xA8);
}

void ps2_8042_disable_second_port() {
    outb(PS2_COMMAND_REGISTER, 0xA7);
}   

void ps2_8042_init_second_port() {
    // PS/2 read command byte
    outb(PS2_COMMAND_REGISTER, 0x20);
    uint8_t command_byte = inb(PS2_DATA_REGISTER);

    // Enable interrupts for second PS/2 port
    command_byte |= 0x02;

    // Write command byte
    outb(PS2_COMMAND_REGISTER, 0x60);
    outb(PS2_DATA_REGISTER, command_byte);
}
