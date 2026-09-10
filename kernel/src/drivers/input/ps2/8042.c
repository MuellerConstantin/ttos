#include <drivers/input/ps2/8042.h>
#include <system/ports.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <util/string.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/types.h>

static device_t* ps2_8042_device = NULL;
static ps2_port_t ps2_8042_ports[2] = { { PS2_FIRST_PORT }, { PS2_SECOND_PORT } };

ps2_port_t* ps2_8042_get_port(uint8_t number) {
    if(number != PS2_FIRST_PORT && number != PS2_SECOND_PORT) {
        return NULL;
    }

    return &ps2_8042_ports[number - 1];
}

device_t* ps2_8042_claim_device(void) {
    // Both ports would otherwise register the controller a second time.
    if(ps2_8042_device) {
        return ps2_8042_device;
    }

    device_t* device = (device_t*) kmalloc(sizeof(device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->name = (char*) kmalloc(16);

    if(!device->name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_generate_id(device->id);
    strcpy(device->name, "PS/2 Controller");

    device->type = DEVICE_TYPE_CONTROLLER;
    device->bus.type = DEVICE_BUS_TYPE_ISA;
    device->bus.data = NULL;
    device->driver.raw = NULL;

    /*
     * The legacy devices sit behind the ISA bridge, so that is where they
     * belong in the tree. Without a bridge they land at the root: they answer
     * on fixed ports either way, so its absence costs the hierarchy only.
     */
    device_register(pci_find_device(PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_ISA_BRIDGE), device);

    ps2_8042_device = device;

    return device;
}

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
