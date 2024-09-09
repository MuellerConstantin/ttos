#include <drivers/pci/pci.h>
#include <sys/kpanic.h>

static linked_list_t* pci_devices = NULL;

static pci_device_t* pci_probe_device(uint8_t bus, uint8_t slot, uint8_t function);
static uint8_t pci_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
static uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
static uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

int32_t pci_init() {
    pci_devices = linked_list_create();

    if(!pci_devices) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    // Scan all slots and functions of the first 10 buses
    for(uint8_t bus = 0; bus < PCI_MAX_DEFAULT_SCAN_BUSES; bus++) {
        for(uint8_t slot = 0; slot < PCI_DEVICES_PER_BUS; slot++) {
            for(uint8_t function = 0; function < PCI_FUNCTIONS_PER_DEVICE; function++) {
                pci_device_t* device = pci_probe_device(bus, slot, function);

                if(!device) {
                    break;
                }

                linked_list_node_t *node = linked_list_create_node(device);

                if(!node) {
                    KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
                }

                linked_list_append(pci_devices, node);

                // Skip function scanning if the device is a single function device
                if(function == 0 && (pci_read_byte(bus, slot, function, PCI_HEADER_TYPE) & 0x80) == 0) {
                    break;
                }
            }
        }
    }
}

linked_list_t* pci_get_devices() {
    return pci_devices;
}

pci_device_t* pci_get_device(uint16_t vendor_id, uint16_t device_id) {
    linked_list_foreach(pci_devices, node) {
        pci_device_t *device = (pci_device_t*) node->data;

        if(device->vendor_id == vendor_id && device->device_id == device_id) {
            return device;
        }
    }

    return NULL;
}

linked_list_t* pci_get_devices_of_type(uint8_t type, int16_t subtype) {
    linked_list_node_t* selected_devices = linked_list_create();

    if(!selected_devices) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    linked_list_foreach(pci_devices, node) {
        pci_device_t *device = (pci_device_t*) node->data;

        if(device->type == type) {
            if(subtype >= 0 && device->subtype != subtype) {
                continue;
            }

            linked_list_node_t *new_node = linked_list_create_node(device);

            if(!new_node) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            linked_list_append(selected_devices, new_node);
        }
    }

    return selected_devices;
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

    pci_device_t *device = (pci_device_t*) kmalloc(sizeof(pci_device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->bus = bus;
    device->slot = slot;
    device->vendor_id = vendor_id;
    device->device_id = device_id;
    device->type = type;
    device->subtype = subtype;
    device->prog_if = prog_if;

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
