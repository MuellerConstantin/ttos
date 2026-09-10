#include <drivers/pci/pci.h>
#include <system/kpanic.h>
#include <memory/kheap.h>
#include <system/kmessage.h>
#include <util/string.h>

/** Length of the buffer a generated device name is written into. */
#define PCI_DEVICE_NAME_LENGTH 64

/** Length of the buffer the kernel message about a found device is built in. */
#define PCI_MESSAGE_LENGTH 128

struct pci_device_class {
    uint8_t type;
    uint8_t subtype;
    const char* name;
};

/*
 * The name a device is registered under, taken from the class it reports
 * rather than from its vendor and device ID: a lookup table for the numeric
 * IDs would have to name every device ever built, while the class is enough to
 * tell the reader of a device listing what the hardware is.
 *
 * Entries are searched in order, so the PCI_SUBTYPE_ANY catch all of a class
 * has to sit behind the subclasses it stands in for.
 */
static const struct pci_device_class pci_device_classes[] = {
    { PCI_TYPE_UNDEFINED, PCI_SUBTYPE_ANY, "Undefined Device" },

    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_SCSI_BUS_CONTROLLER, "SCSI Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_IDE_CONTROLLER, "IDE Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_FLOPPY_DISK_CONTROLLER, "Floppy Disk Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_IPI_BUS_CONTROLLER, "IPI Bus Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_RAID_CONTROLLER, "RAID Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_ATA_CONTROLLER, "ATA Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_SATA_CONTROLLER, "SATA Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_SAS_CONTROLLER, "SAS Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_NVM_CONTROLLER, "NVM Controller" },
    { PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_ANY, "Mass Storage Controller" },

    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_ETHERNET_CONTROLLER, "Ethernet Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_TOKEN_RING_CONTROLLER, "Token Ring Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_FDDI_CONTROLLER, "FDDI Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_ATM_CONTROLLER, "ATM Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_ISDN_CONTROLLER, "ISDN Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_WORLDFIP_CONTROLLER, "WorldFip Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_PICMG_CONTROLLER, "PICMG Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_INFINIBAND_CONTROLLER, "InfiniBand Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_FABRIC_CONTROLLER, "Fabric Controller" },
    { PCI_TYPE_NETWORK_CONTROLLER, PCI_SUBTYPE_ANY, "Network Controller" },

    { PCI_TYPE_DISPLAY_CONTROLLER, PCI_SUBTYPE_VGA_CONTROLLER, "VGA Controller" },
    { PCI_TYPE_DISPLAY_CONTROLLER, PCI_SUBTYPE_XGA_CONTROLLER, "XGA Controller" },
    { PCI_TYPE_DISPLAY_CONTROLLER, PCI_SUBTYPE_3D_CONTROLLER, "3D Controller" },
    { PCI_TYPE_DISPLAY_CONTROLLER, PCI_SUBTYPE_ANY, "Display Controller" },

    { PCI_TYPE_MULTIMEDIA_CONTROLLER, PCI_SUBTYPE_VIDEO_CONTROLLER, "Video Controller" },
    { PCI_TYPE_MULTIMEDIA_CONTROLLER, PCI_SUBTYPE_AUDIO_CONTROLLER, "Audio Controller" },
    { PCI_TYPE_MULTIMEDIA_CONTROLLER, PCI_SUBTYPE_TELEPHONY_CONTROLLER, "Telephony Controller" },
    { PCI_TYPE_MULTIMEDIA_CONTROLLER, PCI_SUBTYPE_AUDIO_DEVICE, "Audio Device" },
    { PCI_TYPE_MULTIMEDIA_CONTROLLER, PCI_SUBTYPE_ANY, "Multimedia Controller" },

    { PCI_TYPE_MEMORY_CONTROLLER, PCI_SUBTYPE_RAM_CONTROLLER, "RAM Controller" },
    { PCI_TYPE_MEMORY_CONTROLLER, PCI_SUBTYPE_FLASH_CONTROLLER, "Flash Controller" },
    { PCI_TYPE_MEMORY_CONTROLLER, PCI_SUBTYPE_ANY, "Memory Controller" },

    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_HOST_BRIDGE, "Host Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_ISA_BRIDGE, "ISA Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_EISA_BRIDGE, "EISA Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_MCA_BRIDGE, "MCA Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_PCI_TO_PCI_BRIDGE, "PCI-to-PCI Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_PCMCIA_BRIDGE, "PCMCIA Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_NUBUS_BRIDGE, "NuBus Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_CARDBUS_BRIDGE, "CardBus Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_RACEWAY_BRIDGE, "RACEway Bridge" },
    { PCI_TYPE_BRIDGE_DEVICE, PCI_SUBTYPE_ANY, "Bridge" },

    { PCI_TYPE_SIMPLE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_SERIAL_CONTROLLER, "Serial Controller" },
    { PCI_TYPE_SIMPLE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_PARALLEL_CONTROLLER, "Parallel Controller" },
    { PCI_TYPE_SIMPLE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_MULTIPORT_SERIAL_CONTROLLER, "Multiport Serial Controller" },
    { PCI_TYPE_SIMPLE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_MODEM, "Modem" },
    { PCI_TYPE_SIMPLE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_GPIB_CONTROLLER, "GPIB Controller" },
    { PCI_TYPE_SIMPLE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_SMARTCARD_CONTROLLER, "Smart Card Controller" },
    { PCI_TYPE_SIMPLE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_ANY, "Communication Controller" },

    { PCI_TYPE_BASE_SYSTEM_PERIPHERAL, PCI_SUBTYPE_PIC, "Interrupt Controller" },
    { PCI_TYPE_BASE_SYSTEM_PERIPHERAL, PCI_SUBTYPE_DMA_CONTROLLER, "DMA Controller" },
    { PCI_TYPE_BASE_SYSTEM_PERIPHERAL, PCI_SUBTYPE_TIMER, "Timer" },
    { PCI_TYPE_BASE_SYSTEM_PERIPHERAL, PCI_SUBTYPE_RTC_CONTROLLER, "Real Time Clock" },
    { PCI_TYPE_BASE_SYSTEM_PERIPHERAL, PCI_SUBTYPE_PCI_HOTPLUG_CONTROLLER, "PCI Hotplug Controller" },
    { PCI_TYPE_BASE_SYSTEM_PERIPHERAL, PCI_SUBTYPE_SD_HOST_CONTROLLER, "SD Host Controller" },
    { PCI_TYPE_BASE_SYSTEM_PERIPHERAL, PCI_SUBTYPE_ANY, "System Peripheral" },

    { PCI_TYPE_INPUT_DEVICE_CONTROLLER, PCI_SUBTYPE_KEYBOARD_CONTROLLER, "Keyboard Controller" },
    { PCI_TYPE_INPUT_DEVICE_CONTROLLER, PCI_SUBTYPE_DIGITIZER_PEN, "Digitizer Pen" },
    { PCI_TYPE_INPUT_DEVICE_CONTROLLER, PCI_SUBTYPE_MOUSE_CONTROLLER, "Mouse Controller" },
    { PCI_TYPE_INPUT_DEVICE_CONTROLLER, PCI_SUBTYPE_SCANNER_CONTROLLER, "Scanner Controller" },
    { PCI_TYPE_INPUT_DEVICE_CONTROLLER, PCI_SUBTYPE_GAMEPORT_CONTROLLER, "Gameport Controller" },
    { PCI_TYPE_INPUT_DEVICE_CONTROLLER, PCI_SUBTYPE_ANY, "Input Device Controller" },

    { PCI_TYPE_DOCKING_STATION, PCI_SUBTYPE_ANY, "Docking Station" },

    { PCI_TYPE_PROCESSOR, PCI_SUBTYPE_386, "386 Processor" },
    { PCI_TYPE_PROCESSOR, PCI_SUBTYPE_486, "486 Processor" },
    { PCI_TYPE_PROCESSOR, PCI_SUBTYPE_PENTIUM, "Pentium Processor" },
    { PCI_TYPE_PROCESSOR, PCI_SUBTYPE_ALPHA, "Alpha Processor" },
    { PCI_TYPE_PROCESSOR, PCI_SUBTYPE_POWERPC, "PowerPC Processor" },
    { PCI_TYPE_PROCESSOR, PCI_SUBTYPE_MIPS, "MIPS Processor" },
    { PCI_TYPE_PROCESSOR, PCI_SUBTYPE_COPROCESSOR, "Coprocessor" },
    { PCI_TYPE_PROCESSOR, PCI_SUBTYPE_ANY, "Processor" },

    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_FIREWIRE_CONTROLLER, "FireWire Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_ACCESS_BUS_CONTROLLER, "ACCESS Bus Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_SSA_CONTROLLER, "SSA Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_USB_CONTROLLER, "USB Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_FIBRE_CHANNEL_CONTROLLER, "Fibre Channel Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_SMBUS_CONTROLLER, "SMBus Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_SERIAL_INFINIBAND_CONTROLLER, "InfiniBand Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_IPMI_CONTROLLER, "IPMI Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_SERCOS_CONTROLLER, "SERCOS Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_CANBUS_CONTROLLER, "CAN Bus Controller" },
    { PCI_TYPE_SERIAL_BUS_CONTROLLER, PCI_SUBTYPE_ANY, "Serial Bus Controller" },

    { PCI_TYPE_WIRELESS_CONTROLLER, PCI_SUBTYPE_IRDA_CONTROLLER, "IrDA Controller" },
    { PCI_TYPE_WIRELESS_CONTROLLER, PCI_SUBTYPE_CONSUMER_IR_CONTROLLER, "Consumer IR Controller" },
    { PCI_TYPE_WIRELESS_CONTROLLER, PCI_SUBTYPE_RF_CONTROLLER, "RF Controller" },
    { PCI_TYPE_WIRELESS_CONTROLLER, PCI_SUBTYPE_BLUETOOTH_CONTROLLER, "Bluetooth Controller" },
    { PCI_TYPE_WIRELESS_CONTROLLER, PCI_SUBTYPE_BROADBAND_CONTROLLER, "Broadband Controller" },
    { PCI_TYPE_WIRELESS_CONTROLLER, PCI_SUBTYPE_WIRELESS_ETHERNET_CONTROLLER, "Wireless Ethernet Controller" },
    { PCI_TYPE_WIRELESS_CONTROLLER, PCI_SUBTYPE_ANY, "Wireless Controller" },

    { PCI_TYPE_INTELLIGENT_IO_CONTROLLER, PCI_SUBTYPE_ANY, "Intelligent IO Controller" },

    { PCI_TYPE_SATELLITE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_SATELLITE_TV_CONTROLLER, "Satellite TV Controller" },
    { PCI_TYPE_SATELLITE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_SATELLITE_AUDIO_CONTROLLER, "Satellite Audio Controller" },
    { PCI_TYPE_SATELLITE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_SATELLITE_VOICE_CONTROLLER, "Satellite Voice Controller" },
    { PCI_TYPE_SATELLITE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_SATELLITE_DATA_CONTROLLER, "Satellite Data Controller" },
    { PCI_TYPE_SATELLITE_COMMUNICATIONS_CONTROLLER, PCI_SUBTYPE_ANY, "Satellite Controller" },

    { PCI_TYPE_ENCRYPTION_CONTROLLER, PCI_SUBTYPE_ENCRYPTION_NETWORK_CONTROLLER, "Network Encryption Controller" },
    { PCI_TYPE_ENCRYPTION_CONTROLLER, PCI_SUBTYPE_ENTERTAINMENT_CONTROLLER, "Entertainment Encryption Controller" },
    { PCI_TYPE_ENCRYPTION_CONTROLLER, PCI_SUBTYPE_ANY, "Encryption Controller" },

    { PCI_TYPE_SIGNAL_PROCESSING_CONTROLLER, PCI_SUBTYPE_DPIO_CONTROLLER, "DPIO Module" },
    { PCI_TYPE_SIGNAL_PROCESSING_CONTROLLER, PCI_SUBTYPE_ANY, "Signal Processing Controller" },

    { PCI_TYPE_PROCESSING_ACCELERATOR, PCI_SUBTYPE_ANY, "Processing Accelerator" },
    { PCI_TYPE_NON_ESSENTIAL_INSTRUMENTATION, PCI_SUBTYPE_ANY, "Non-Essential Instrumentation" },
    { PCI_TYPE_COPROCESSOR, PCI_SUBTYPE_ANY, "Coprocessor" },
};

static const char* pci_get_class_name(uint8_t type, uint8_t subtype);
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
            for(uint8_t function = 0; function < PCI_FUNCTIONS_PER_DEVICE; function++) {
                pci_device_t* pci_device = pci_probe_device(bus, slot, function);

                /*
                 * The function numbers of a multi function device need not be
                 * contiguous. An absent function therefore only means that this
                 * one is missing, not that the ones behind it are: the PIIX3
                 * for instance answers on functions 0, 1 and 3.
                 */
                if(!pci_device) {
                    if(function == 0) {
                        break;
                    }

                    continue;
                }

                device_t* device = (device_t*) kmalloc(sizeof(device_t));

                if(!device) {
                    KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
                }

                device_generate_id(device->id);
                device->name = pci_get_device_name(pci_device);
                device->type = DEVICE_TYPE_UNKNOWN;
                device->bus.type = DEVICE_BUS_TYPE_PCI;
                device->bus.data = pci_device;

                // Nobody drives the device yet, the scan only reports that it exists.
                device->driver.raw = NULL;

                /*
                 * The functions of a multi function device sit next to each
                 * other on the bus and share nothing but their address. None of
                 * them is above the others, so they are registered at the same
                 * level rather than below function zero.
                 */
                device_register(NULL, device);

                char* kernel_message = (char*) kmalloc(PCI_MESSAGE_LENGTH);

                if(!kernel_message) {
                    KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
                }

                strfmt(kernel_message, "pci: Found %s, Vendor ID: %x, Device ID: %x", device->name, pci_device->vendor_id, pci_device->device_id);

                kmessage(KMESSAGE_LEVEL_INFO, kernel_message);

                // Skip function scanning if the device is a single function device
                if(function == 0 && (pci_device->header_type & PIC_HEADER_TYPE_MULTIFUNCTION) == 0) {
                    break;
                }
            }
        }
    }

    return 0;
}

device_t* pci_find_device(uint8_t type, uint8_t subtype) {
    linked_list_t* devices = pci_find_all_devices(type, subtype);

    if(!devices) {
        return NULL;
    }

    device_t* result = devices->head ? (device_t*) devices->head->data : NULL;

    linked_list_destroy(devices, false);

    return result;
}

linked_list_t* pci_find_all_devices(uint8_t type, uint8_t subtype) {
    linked_list_t* devices = (linked_list_t*) device_find_all_by_bus_type(DEVICE_BUS_TYPE_PCI);
    linked_list_t* result = linked_list_create();

    if(!result) {
        linked_list_destroy(devices, false);

        return NULL;
    }

    linked_list_foreach(devices, node) {
        device_t* device = (device_t*) node->data;
        pci_device_t* pci_device = (pci_device_t*) device->bus.data;

        if(pci_device->type != type || pci_device->subtype != subtype) {
            continue;
        }

        linked_list_node_t* match = linked_list_create_node(device);

        if(!match) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        linked_list_append(result, match);
    }

    linked_list_destroy(devices, false);

    return result;
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

static const char* pci_get_class_name(uint8_t type, uint8_t subtype) {
    for(size_t i = 0; i < sizeof(pci_device_classes) / sizeof(pci_device_classes[0]); i++) {
        const struct pci_device_class* device_class = &pci_device_classes[i];

        if(device_class->type != type) {
            continue;
        }

        if(device_class->subtype == subtype || device_class->subtype == PCI_SUBTYPE_ANY) {
            return device_class->name;
        }
    }

    return "PCI Device";
}

static char* pci_get_device_name(pci_device_t* pci_device) {
    char* name = (char*) kmalloc(PCI_DEVICE_NAME_LENGTH);

    if(!name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    /*
     * The address is part of the name because the class alone does not tell
     * two devices of the same kind apart, and a machine with two IDE channels
     * or two network cards is nothing unusual.
     */
    strfmt(name, "%s (%d:%d.%d)", pci_get_class_name(pci_device->type, pci_device->subtype), pci_device->bus, pci_device->slot, pci_device->function);

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