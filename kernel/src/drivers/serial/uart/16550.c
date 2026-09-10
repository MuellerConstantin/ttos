#include <drivers/serial/uart/16550.h>
#include <system/ports.h>
#include <stdbool.h>
#include <device/device.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/types.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <util/string.h>

static const uint16_t UART_16550_PORTS[] = {
    UART_16550_COM1, UART_16550_COM2, UART_16550_COM3, UART_16550_COM4,
    UART_16550_COM5, UART_16550_COM6, UART_16550_COM7, UART_16550_COM8
};

static bool uart_16550_probe(uint16_t port);
static char* uart_16550_get_device_name(uint16_t port);

static int8_t uart_16550_flushable(uint16_t port);
static void uart_16550_write_byte(uint16_t port, uint8_t data);

static int8_t uart_16550_available(uint16_t port);
static uint8_t uart_16550_read_byte(uint16_t port);

int32_t uart_16550_init(uint16_t port, uint32_t baud_rate) {
    if (!uart_16550_probe(port)) {
        return -1;
    }

    outb(UART_16550_IER(port), 0x00); // Disable all interrupts

    outb(UART_16550_LCR(port), 0x80); // Enable DLAB (Baud rate divisor)
    outb(UART_16550_DLL(port), UART_16550_BAUD_RATE_LSB(baud_rate)); // Set divisor's least significant byte for baud rate
    outb(UART_16550_DLM(port), UART_16550_BAUD_RATE_MSB(baud_rate)); // Set divisor's most significant byte for baud rate

    outb(UART_16550_LCR(port), 0x03); // 8 bits, no parity, one stop bit
    outb(UART_16550_IIR(port), 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(UART_16550_MCR(port), 0x0B); // IRQs enabled, RTS/DSR set

    device_t* device = (device_t*) kmalloc(sizeof(device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_generate_id(device->id);

    device->name = uart_16550_get_device_name(port);
    device->type = DEVICE_TYPE_SERIAL;
    device->bus.type = DEVICE_BUS_TYPE_ISA;
    device->bus.data = NULL;

    // Nothing writes to the port yet, so there is no driver to bind.
    device->driver.raw = NULL;

    /*
     * The legacy devices sit behind the ISA bridge, so that is where they
     * belong in the tree. Without a bridge they land at the root: the port
     * answers on its fixed address either way.
     */
    device_register(pci_find_device(PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_ISA_BRIDGE), device);

    return 0;
}

static char* uart_16550_get_device_name(uint16_t port) {
    char* name = (char*) kmalloc(20);

    if(!name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    for(uint8_t index = 0; index < sizeof(UART_16550_PORTS) / sizeof(UART_16550_PORTS[0]); index++) {
        if(UART_16550_PORTS[index] == port) {
            strfmt(name, "Serial Port COM%d", index + 1);

            return name;
        }
    }

    strfmt(name, "Serial Port %x", port);

    return name;
}

static bool uart_16550_probe(uint16_t port) {
    // Enable loopback mode for self-test
    outb(UART_16550_MCR(port), 0x1E);
    outb(port, 0xAE);

    if(inb(port) != 0xAE) {
        return false;
    }

    outb(UART_16550_MCR(port), 0x0F);
    return true;
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
