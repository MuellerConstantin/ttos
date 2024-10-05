#include <drivers/storage/sata.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>
#include <device/device.h>
#include <drivers/pci/pci.h>
#include <memory/map.h>
#include <memory/paging.h>

int32_t sata_init() {
    linked_list_t* devices = device_find_all_by_bus_type(DEVICE_BUS_TYPE_PCI);

    linked_list_foreach(devices, node) {
        device_t* device = (device_t*) node->data;
        pci_device_t* pci_device = (pci_device_t*) device->bus.data;

        if(pci_device->type == PCI_TYPE_MASS_STORAGE_CONTROLLER && pci_device->subtype == PCI_SUBTYPE_SATA_CONTROLLER) {
            uint32_t bar5_address = pci_get_bar_address(pci_device, 5);
            size_t bar5_size = pci_get_bar_size(pci_device, 5);
            uint8_t interrupt_line = pci_get_interrupt_line(pci_device);

            if(bar5_size > SATA_DMA_BUFFER_VIRTUAL_SIZE) {
                return -1;
            }

            paging_map_memory((void*) SATA_DMA_BUFFER_VIRTUAL_BASE, bar5_size, (void*) bar5_address, true, true);

            pci_device->data.general.bar[5] = SATA_DMA_BUFFER_VIRTUAL_BASE;
            pci_device->data.general.bar_size[5] = bar5_size;
            pci_device->data.general.interrupt_line = interrupt_line;

            char* new_device_name = (char*) kmalloc(21);

            if(!new_device_name) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            strcpy(new_device_name, "AHCI SATA Controller");

            kfree(device->name);
            device->name = new_device_name;

            break;
        }
    }

    linked_list_destroy(devices, false);

    return 0;
}
