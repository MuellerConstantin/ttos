#ifndef _KERNEL_PLATFORM_INTEL_DRIVERS_UART_16550_H
#define _KERNEL_PLATFORM_INTEL_DRIVERS_UART_16550_H

#include <stdint.h>
#include <stddef.h>

#define UART_16550_COM1 0x3F8
#define UART_16550_COM2 0x2F8
#define UART_16550_COM3 0x3E8
#define UART_16550_COM4 0x2E8
#define UART_16550_COM5 0x5F8
#define UART_16550_COM6 0x4F8
#define UART_16550_COM7 0x5E8
#define UART_16550_COM8 0x4E8

#define UART_16550_MAX_BAUD_RATE 115200

#define UART_16550_BAUD_RATE_LSB(baud_rate) (UART_16550_MAX_BAUD_RATE / baud_rate) & 0xFF
#define UART_16550_BAUD_RATE_MSB(baud_rate) ((UART_16550_MAX_BAUD_RATE / baud_rate) >> 8) & 0xFF

#define UART_16550_RBR(port) (port + 0x00) // Receive Buffer Register
#define UART_16550_THR(port) (port + 0x00) // Transmit Holding Register
#define UART_16550_DLL(port) (port + 0x00) // Divisor Least Significant Byte
#define UART_16550_IER(port) (port + 0x01) // Interrupt Enable Register
#define UART_16550_DLM(port) (port + 0x01) // Divisor Most Significant Byte
#define UART_16550_IIR(port) (port + 0x02) // Interrupt Identification Register
#define UART_16550_FCR(port) (port + 0x02) // FIFO Control Register
#define UART_16550_LCR(port) (port + 0x03) // Line Control Register
#define UART_16550_MCR(port) (port + 0x04) // Modem Control Register
#define UART_16550_LSR(port) (port + 0x05) // Line Status Register
#define UART_16550_MSR(port) (port + 0x06) // Modem Status Register
#define UART_16550_SCR(port) (port + 0x07) // Scratch Register

/**
 * Initializes a 16550 UART serial port.
 * 
 * @param port The I/O port of the UART serial port.
 * @param baud_rate The baud rate of the UART serial port.
 */
void uart_16550_init(uint16_t port, uint32_t baud_rate);

/**
 * Writes data to a 16550 UART serial port.
 * 
 * @param port The I/O port of the UART serial port.
 * @param data A pointer to the data to write.
 * @param size The number of bytes to write.
 */
void uart_16550_write(uint16_t port, const void* data, size_t size);

/**
 * Reads data from a 16550 UART serial port.
 * 
 * @param port The I/O port of the UART serial port.
 * @param buffer A pointer to the buffer to read into.
 * @param size The number of bytes to read.
 */
void uart_16550_read(uint16_t port, void* buffer, size_t size);

#endif // _KERNEL_PLATFORM_INTEL_DRIVERS_UART_16550_H
