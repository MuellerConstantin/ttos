#include <device/volume.h>
#include <string.h>
#include <fs/mbr.h>
#include <sys/kpanic.h>
#include <drivers/serial/uart/16550.h>

static linked_list_t* volumes;

static bool volume_unregister_device_compare(void* node_data, void* compare_data);
static bool volume_find_by_id_compare(void* node_data, void* compare_data);

void volume_init() {
    volumes = linked_list_create();

    if(!volumes) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }
}

linked_list_t* volume_get_all() {
    return volumes;
}

size_t volume_register_device(storage_device_t* device) {
    if(!mbr_probe(device)) {
        return 0;
    }

    mbr_t* mbr = mbr_read(device);

    // Check for partitions
    for(size_t i = 0; i < 4; i++) {
        mbr_partition_entry_t* partition = &mbr->partitions[i];

        // Check if partition is present
        if(partition->type == 0) {
            continue;
        }

        volume_t* volume = (volume_t*) kmalloc(sizeof(volume_t));

        if(!volume) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        generate_uuid_v4(&volume->id);

        volume->name = (char*) kmalloc(strlen(device->info.name) + 4);

        if(!volume->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        strcpy(volume->name, device->info.name);
        strcpy(volume->name + strlen(device->info.name), " #");

        char partition_number[2];
        itoa(i + 1, partition_number, 10);

        strcpy(volume->name + strlen(device->info.name) + 2, partition_number);

        volume->device = device;

        linked_list_node_t* new_node = linked_list_create_node(volume);

        if(!new_node) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        linked_list_append(volumes, new_node);
    }

    return 0;
}

static bool volume_unregister_device_compare(void* node_data, void* compare_data) {
    volume_t* volume = (volume_t*) node_data;
    storage_device_t* device = (storage_device_t*) compare_data;

    return volume->device == device;
}

void volume_unregister_device(storage_device_t* device) {
    linked_list_node_t* node = linked_list_find(volumes, volume_unregister_device_compare, device);

    if(!node) {
        return;
    }

    linked_list_remove(volumes, node);

    volume_t* volume = (volume_t*) node->data;

    kfree(volume->name);
    kfree(volume);
    kfree(node);
}

int32_t volume_mount(char drive, volume_t* volume) {
    return -1;
}

int32_t volume_unmount(volume_t* volume) {
    return -1;
}

static bool volume_find_by_id_compare(void* node_data, void* compare_data) {
    volume_t* volume = (volume_t*) node_data;
    uuid_t* id = (uuid_t*) compare_data;

    return uuid_v4_compare(&volume->id, id) == 0;
}

volume_t* volume_find_by_id(uuid_t id) {
    linked_list_node_t* node = linked_list_find(volumes, volume_find_by_id_compare, &id);

    if(!node) {
        return NULL;
    }

    return (volume_t*) node->data;
}
