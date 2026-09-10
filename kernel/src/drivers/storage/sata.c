#include <drivers/storage/sata.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <device/device.h>
#include <drivers/pci/pci.h>
#include <memory/vmm.h>

int32_t sata_init() {
    device_t* device = pci_find_device(PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_SATA_CONTROLLER);

    if(!device) {
        return 0;
    }

    if(pci_load_bar_info((pci_device_t*) device->bus.data, 5) != 0) {
        return -1;
    }

    pci_device_t* pci_device = (pci_device_t*) device->bus.data;

    void* bar5_virtual_base = vmm_map_memory(NULL, pci_device->data.general.bar[5].size, (void*) pci_device->data.general.bar[5].base_address, true, true);

    if(!bar5_virtual_base) {
        return -1;
    }

    char* new_device_name = (char*) kmalloc(21);

    if(!new_device_name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(new_device_name, "AHCI SATA Controller");

    kfree(device->name);

    device->name = new_device_name;
    device->type = DEVICE_TYPE_CONTROLLER;

    return 0;
}
