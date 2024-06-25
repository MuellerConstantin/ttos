#ifndef _KERNEL_DRIVERS_PIC_8259_H
#define _KERNEL_DRIVERS_PIC_8259_H

#include <stdint.h>

#define PIC_8259_MASTER_COMMAND_REGISTER  0x20
#define PIC_8259_MASTER_DATA_REGISTER     0x21

#define PIC_8259_SLAVE_COMMAND_REGISTER     0xA0
#define PIC_8259_SLAVE_DATA_REGISTER        0xA1

#define PIC_8259_EOI 0x20

#define PIC_8259_ICW1_ICW4 0x01
#define PIC_8259_ICW1_SINGLE 0x02
#define PIC_8259_ICW1_INTERVAL4 0x04
#define PIC_8259_ICW1_LEVEL 0x08
#define PIC_8259_ICW1_INIT 0x10

#define PIC_8259_ICW4_8086 0x01
#define PIC_8259_ICW4_AUTO 0x02
#define PIC_8259_ICW4_BUF_SLAVE 0x08
#define PIC_8259_ICW4_BUF_MASTER 0x0C
#define PIC_8259_ICW4_SFNM 0x10

/**
 * Initialize the 8259 PIC.
 */
void pic_8259_init();

/**
 * Send End of Interrupt (EOI) to the PIC
 * 
 * @param irq IRQ number.
 */
void pic_8259_send_eoi(uint8_t irq);

/**
 * Mask an IRQ. This will prevent the PIC from sending the IRQ to the CPU.
 * 
 * @param irq IRQ number.
 */
void pic_8259_mask_irq(uint8_t irq);

/**
 * Unmask an IRQ. This will allow the PIC to send the IRQ to the CPU.
 * 
 * @param irq IRQ number.
 */
void pic_8259_unmask_irq(uint8_t irq);

#endif // _KERNEL_DRIVERS_PIC_8259_H
