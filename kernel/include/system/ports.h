/**
 * @file ports.h
 * @brief Definitions for I/O ports.
 * 
 * This file contains definitions for I/O ports and functions to read and write to them.
 */

#ifndef _KERNEL_SYSTEM_PORTS_H
#define _KERNEL_SYSTEM_PORTS_H

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

/**
 * Writes a word to an I/O port.
 * 
 * @param port The I/O port to write to.
 * @param data The word to write.
 */
void outw(uint16_t port, uint16_t data);

/**
 * Reads a word from an I/O port.
 * 
 * @param port The I/O port to read from.
 * @return The word read.
 */
uint16_t inw(uint16_t port);

/**
 * Writes a double word to an I/O port.
 * 
 * @param port The I/O port to write to.
 * @param data The double word to write.
 */
void outl(uint16_t port, uint32_t data);

/**
 * Reads a double word from an I/O port.
 * 
 * @param port The I/O port to read from.
 * @return The double word read.
 */
uint32_t inl(uint16_t port);

#endif // _KERNEL_SYSTEM_PORTS_H
