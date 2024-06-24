#include <drivers/pic/8259.h>
#include <io/ports.h>

void pic_8259_init() {
    uint8_t mask1 = inb(PIC_8259_MASTER_DATA_REGISTER);
    uint8_t mask2 = inb(PIC_8259_SLAVE_DATA_REGISTER);

    // ICW1: Initialize remapping for maser + slave by sending ICW1 (Initialisation Command Word)

    outb(PIC_8259_MASTER_COMMAND_REGISTER, PIC_8259_ICW1_INIT | PIC_8259_ICW1_ICW4);
    outb(PIC_8259_SLAVE_COMMAND_REGISTER, PIC_8259_ICW1_INIT | PIC_8259_ICW1_ICW4);

    // ICW2: Set PIC offset for master + slave

    outb(PIC_8259_MASTER_DATA_REGISTER, 0x20);
    outb(PIC_8259_SLAVE_DATA_REGISTER, 0x28);

    // ICW3: Set master + slave PIC connection

    outb(PIC_8259_MASTER_DATA_REGISTER, 0x04);
    outb(PIC_8259_SLAVE_DATA_REGISTER, 0x02);

    // ICW4: Set master + slave PIC mode

    outb(PIC_8259_MASTER_DATA_REGISTER, PIC_8259_ICW4_8086);
    outb(PIC_8259_SLAVE_DATA_REGISTER, PIC_8259_ICW4_8086);

    // Restore masks

    outb(PIC_8259_MASTER_DATA_REGISTER, mask1);
    outb(PIC_8259_SLAVE_DATA_REGISTER, mask2);

    // Unmask all IRQs
    for (uint8_t i = 0; i < 16; i++) {
        pic_8259_unmask_irq(i);
    }
}

void pic_8259_send_eoi(uint8_t irq) {
    if(irq >= 8) {
        outb(PIC_8259_SLAVE_COMMAND_REGISTER, PIC_8259_EOI);
    }

    outb(PIC_8259_MASTER_COMMAND_REGISTER, PIC_8259_EOI);
}

void pic_8259_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = PIC_8259_MASTER_DATA_REGISTER;
    } else {
        port = PIC_8259_SLAVE_DATA_REGISTER;
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

void pic_8259_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = PIC_8259_MASTER_DATA_REGISTER;
    } else {
        port = PIC_8259_SLAVE_DATA_REGISTER;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}
