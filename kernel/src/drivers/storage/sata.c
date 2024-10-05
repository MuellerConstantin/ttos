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
            if(pci_load_bar_info(pci_device, 5) != 0) {
                return -1;
            }

            if(pci_device->data.general.bar[5].size > SATA_DMA_BUFFER_VIRTUAL_SIZE) {
                return -1;
            }

            paging_map_memory((void*) SATA_DMA_BUFFER_VIRTUAL_BASE, pci_device->data.general.bar[5].size, (void*) pci_device->data.general.bar[5].base_address, true, true);

            char* new_device_name = (char*) kmalloc(21);

            if(!new_device_name) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            strcpy(new_device_name, "AHCI SATA Controller");

            kfree(device->name);
            device->name = new_device_name;
            device->type = DEVICE_TYPE_CONTROLLER;

            break;
        }
    }

    linked_list_destroy(devices, false);

    return 0;
}
