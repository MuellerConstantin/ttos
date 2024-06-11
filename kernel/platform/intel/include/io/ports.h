#ifndef _KERNEL_PLATFORM_INTEL_IO_PORTS_H
#define _KERNEL_PLATFORM_INTEL_IO_PORTS_H

#include <stdint.h>

/**
 * Writes a byte to an I/O port.
 * 
 * @param port The I/O port to write to.
 * @param data The byte to write.
 */
void outb(uint16_t port, uint8_t data);

/**
 * Reads a byte from an I/O port.
 * 
 * @param port The I/O port to read from.
 * @return The byte read.
 */
uint8_t inb(uint16_t port);

#endif // _KERNEL_PLATFORM_INTEL_IO_PORTS_H
