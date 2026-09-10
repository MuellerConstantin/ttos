#include <drivers/pit/8253.h>
#include <arch/i386/isr.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/types.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <util/string.h>

static device_t* pit_8253_device = NULL;

int32_t pit_8253_init(uint8_t counter, uint32_t frequency) {
    uint32_t divisor = PIT_8253_OSCILLATOR_FREQUENCY / frequency;

    uint16_t data_register = 0;

    switch (counter) {
        case PIT_8253_COUNTER_0:
            data_register = PIT_8253_COUNTER_0_DATA_REGISTER;
            break;
        case PIT_8253_COUNTER_1:
            data_register = PIT_8253_COUNTER_1_DATA_REGISTER;
            break;
        case PIT_8253_COUNTER_2:
            data_register = PIT_8253_COUNTER_2_DATA_REGISTER;
            break;
        default:
            return -1;
    }

    outb(PIT_8253_COMMAND_REGISTER, 0x36);
    outb(data_register, divisor & 0xFF);
    outb(data_register, divisor >> 8);

    return 0;
}

device_t* pit_8253_claim_device(void) {
    if(pit_8253_device) {
        return pit_8253_device;
    }

    device_t* device = (device_t*) kmalloc(sizeof(device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->name = (char*) kmalloc(28);

    if(!device->name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_generate_id(device->id);
    strcpy(device->name, "Programmable Interval Timer");

    device->type = DEVICE_TYPE_TIMER;
    device->bus.type = DEVICE_BUS_TYPE_ISA;
    device->bus.data = NULL;

    // The timer is reached through timer.c, not through a driver interface.
    device->driver.raw = NULL;

    /*
     * The legacy devices sit behind the ISA bridge, so that is where they
     * belong in the tree. Without a bridge they land at the root.
     */
    device_register(pci_find_device(PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_ISA_BRIDGE), device);

    pit_8253_device = device;

    return device;
}
