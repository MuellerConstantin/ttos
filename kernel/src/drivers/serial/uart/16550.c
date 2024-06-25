#include <drivers/serial/uart/16550.h>
#include <io/ports.h>

static int8_t uart_16550_flushable(uint16_t port);
static void uart_16550_write_byte(uint16_t port, uint8_t data);

static int8_t uart_16550_available(uint16_t port);
static uint8_t uart_16550_read_byte(uint16_t port);

void uart_16550_init(uint16_t port, uint32_t baud_rate) {
    outb(UART_16550_IER(port), 0x00); // Disable all interrupts

    outb(UART_16550_LCR(port), 0x80); // Enable DLAB (Baud rate divisor)
    outb(UART_16550_DLL(port), UART_16550_BAUD_RATE_LSB(baud_rate)); // Set divisor's least significant byte for baud rate
    outb(UART_16550_DLM(port), UART_16550_BAUD_RATE_MSB(baud_rate)); // Set divisor's most significant byte for baud rate

    outb(UART_16550_LCR(port), 0x03); // 8 bits, no parity, one stop bit
    outb(UART_16550_IIR(port), 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(UART_16550_MCR(port), 0x0B); // IRQs enabled, RTS/DSR set
}

static int8_t uart_16550_flushable(uint16_t port) {
    return inb(UART_16550_LSR(port)) & 0x20;
}

static void uart_16550_write_byte(uint16_t port, uint8_t data) {
    while (!uart_16550_flushable(port));
    outb(port, data);
}

void uart_16550_write(uint16_t port, const void* data, size_t size) {
    const uint8_t* data_ptr = (const uint8_t *) data;

    for (size_t index = 0; index < size; index++) {
        uart_16550_write_byte(port, data_ptr[index]);
    }
}

static int8_t uart_16550_available(uint16_t port) {
    return inb(UART_16550_LSR(port)) & 0x01;
}

static uint8_t uart_16550_read_byte(uint16_t port) {
    while (!uart_16550_available(port));
    return inb(port);
}

void uart_16550_read(uint16_t port, void* buffer, size_t size) {
    uint8_t* buffer_ptr = (uint8_t *) buffer;

    for (size_t index = 0; index < size; index++) {
        buffer_ptr[index] = uart_16550_read_byte(port);
    }
}
