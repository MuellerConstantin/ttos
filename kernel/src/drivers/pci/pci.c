#include <drivers/pci/pci.h>
#include <system/kpanic.h>
#include <memory/kheap.h>
#include <system/kmessage.h>
#include <stdio.h>

static char* pci_get_device_name(pci_device_t* pci_device);
static pci_device_t* pci_probe_device(uint8_t bus, uint8_t slot, uint8_t function);
static int32_t pci_general_load_bar_info(pci_device_t* pci_device, uint8_t bar_index);
static int32_t pci_pci2pci_load_bar_info(pci_device_t* pci_device, uint8_t bar_index);

static uint8_t pci_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
static uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
static uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
static void pci_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);

int32_t pci_init() {
    for(uint16_t bus = 0; bus < PCI_MAX_NUM_BUSES; bus++) {
        for(uint8_t slot = 0; slot < PCI_DEVICES_PER_BUS; slot++) {
            device_t* slot_device = NULL;

            for(uint8_t function = 0; function < PCI_FUNCTIONS_PER_DEVICE; function++) {
                pci_device_t* pci_device = pci_probe_device(bus, slot, function);

                if(!pci_device) {
                    break;
                }

                device_t* device = (device_t*) kmalloc(sizeof(device_t));

                if(!device) {
                    KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
                }

                generate_uuid_v4(&device->id);
                device->name = pci_get_device_name(pci_device);
                device->type = DEVICE_TYPE_UNKNOWN;
                device->bus.type = DEVICE_BUS_TYPE_PCI;
                device->bus.data = pci_device;

                if(!slot_device) {
                    slot_device = device;
                    device_register(NULL, slot_device);
                } else {
                    device_register(slot_device, device);
                }

                char* kernel_message = (char*) kmalloc(64);

                if(!kernel_message) {
                    KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
                }

                strfmt(kernel_message, "pci: Found device Vendor ID: %x, Device ID: %x at %d:%d.%d", pci_device->vendor_id, pci_device->device_id, bus, slot, function);

                kmessage(KMESSAGE_LEVEL_INFO, kernel_message);

                // Skip function scanning if the device is a single function device
                if(function == 0 && (pci_device->header_type & PIC_HEADER_TYPE_MULTIFUNCTION) == 0) {
                    break;
                }
            }
        }
    }
}

int32_t pci_load_bar_info(pci_device_t* pci_device, uint8_t bar_index) {
    uint8_t header_type = pci_device->header_type & 0x7F;

    switch(header_type) {
        case PCI_HEADER_TYPE_0:
            return pci_general_load_bar_info(pci_device, bar_index);
        case PCI_HEADER_TYPE_1:
            return pci_pci2pci_load_bar_info(pci_device, bar_index);
        case PCI_HEADER_TYPE_2:
        default:
            return -1;
    }
}

static int32_t pci_general_load_bar_info(pci_device_t* pci_device, uint8_t bar_index) {
    uint8_t offset;

    switch(bar_index) {
        case 0:
            offset = PCI_H0_BAR0;
            break;
        case 1:
            offset = PCI_H0_BAR1;
            break;
        case 2:
            offset = PCI_H0_BAR2;
            break;
        case 3:
            offset = PCI_H0_BAR3;
            break;
        case 4:
            offset = PCI_H0_BAR4;
            break;
        case 5:
            offset = PCI_H0_BAR5;
            break;
        default:
            return -1;
    }

    uint32_t bar_address = pci_read_dword(pci_device->bus, pci_device->slot, pci_device->function, offset);

    if(bar_address & PCI_BAR_IO_SPACE) {
        pci_device->data.general.bar[bar_index].type = PCI_BAR_IO_SPACE;

        // Get BAR size through BAR masking

        pci_write_dword(pci_device->bus, pci_device->slot, pci_device->function, offset, 0xFFFFFFFF);

        uint32_t mask = pci_read_dword(pci_device->bus, pci_device->slot, pci_device->function, offset);

        pci_write_dword(pci_device->bus, pci_device->slot, pci_device->function, offset, bar_address);

        // Apply size and io port

        pci_device->data.general.bar[bar_index].size = ~(mask & ~0x3) + 1;
        pci_device->data.general.bar[bar_index].io_port = bar_address & ~0x3;
        pci_device->data.general.bar[bar_index].flags = bar_address & 0x3;

    } else {
        uint8_t bar_address_type = bar_address & 0x3;

        if(bar_address_type != PCI_BAR_MEMORY_32BIT) {
            return -1;
        }

        pci_device->data.general.bar[bar_index].type = PCI_BAR_MEMORY_SPACE;

        // Get BAR size through BAR masking

        pci_write_dword(pci_device->bus, pci_device->slot, pci_device->function, offset, 0xFFFFFFFF);

        uint32_t mask = pci_read_dword(pci_device->bus, pci_device->slot, pci_device->function, offset);

        pci_write_dword(pci_device->bus, pci_device->slot, pci_device->function, offset, bar_address);

        // Apply size and base address

        pci_device->data.general.bar[bar_index].size = ~(mask & ~0xf) + 1;
        pci_device->data.general.bar[bar_index].base_address = bar_address;
        pci_device->data.general.bar[bar_index].flags = bar_address & 0xf;
    }

    return 0;
}

static int32_t pci_pci2pci_load_bar_info(pci_device_t* pci_device, uint8_t bar_index) {
    return -1;
}

static char* pci_get_device_name(pci_device_t* pci_device) {
    char* name = (char*) kmalloc(32);

    if(!name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    /*
     * The following code for generating the device name is not
     * the most efficient way to do it, but it is simple way that
     * works without strfmt.
     */

    char* name_ptr = name;

    strcpy(name_ptr, "PCI Device ");
    name_ptr += 11;

    char vendor_id_buffer[5];

    itoa(pci_device->vendor_id, vendor_id_buffer, 16);
    strcpy(name_ptr, vendor_id_buffer);
    name_ptr += strlen(vendor_id_buffer);

    *name_ptr = '/';
    name_ptr++;

    char device_id_buffer[5];

    itoa(pci_device->device_id, device_id_buffer, 16);
    strcpy(name_ptr, device_id_buffer);
    name_ptr += strlen(device_id_buffer);

    *name_ptr = ' ';
    name_ptr++;

    *name_ptr = '(';
    name_ptr++;

    char bus_buffer[3];
    
    itoa(pci_device->bus, bus_buffer, 16);
    strcpy(name_ptr, bus_buffer);
    name_ptr += strlen(bus_buffer);

    *name_ptr = ':';
    name_ptr++;

    char slot_buffer[3];

    itoa(pci_device->slot, slot_buffer, 16);
    strcpy(name_ptr, slot_buffer);
    name_ptr += strlen(slot_buffer);

    *name_ptr = '.';
    name_ptr++;

    char function_buffer[3];

    itoa(pci_device->function, function_buffer, 16);
    strcpy(name_ptr, function_buffer);
    name_ptr += strlen(function_buffer);

    *name_ptr = ')';
    name_ptr++;

    *name_ptr = '\0';

    return name;
}

static pci_device_t* pci_probe_device(uint8_t bus, uint8_t slot, uint8_t function) {
    uint16_t vendor_id = pci_read_word(bus, slot, function, PCI_VENDOR_ID);

    if(vendor_id == 0xFFFF) {
        return NULL;
    }

    uint16_t device_id = pci_read_word(bus, slot, function, PCI_DEVICE_ID);
    uint8_t type = pci_read_byte(bus, slot, function, PCI_CLASS);
    uint8_t subtype = pci_read_byte(bus, slot, function, PCI_SUBCLASS);
    uint8_t prog_if = pci_read_byte(bus, slot, function, PCI_PROG_IF);
    uint8_t header_type = pci_read_byte(bus, slot, function, PCI_HEADER_TYPE);

    pci_device_t *device = (pci_device_t*) kmalloc(sizeof(pci_device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->bus = bus;
    device->slot = slot;
    device->function = function;
    device->vendor_id = vendor_id;
    device->device_id = device_id;
    device->type = type;
    device->subtype = subtype;
    device->prog_if = prog_if;
    device->header_type = header_type;

    return device;
}

static uint8_t pci_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t) ((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t) 0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);

    return (uint8_t) ((inl(PCI_CONFIG_DATA) >> ((offset & 3) * 8)) & 0xFF);
}

static uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t) ((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t) 0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);

    return (uint16_t) ((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}

static uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t) ((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t) 0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);

    return (uint32_t) (inl(PCI_CONFIG_DATA));
}

static void pci_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t) ((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t) 0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}