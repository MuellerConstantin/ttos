#include <device/volume.h>
#include <string.h>
#include <fs/mbr.h>
#include <sys/kpanic.h>

static linked_list_t* volumes;

static bool volume_unregister_device_compare(void* node_data, void* compare_data);
static bool volume_find_by_id_compare(void* node_data, void* compare_data);
static bool volume_find_by_name_compare(void* node_data, void* compare_data);
static size_t volume_total_size(volume_t* volume);
static size_t volume_read(volume_t* volume, size_t offset, size_t size, char* buffer);
static size_t volume_write(volume_t* volume, size_t offset, size_t size, char* buffer);

void volume_init() {
    volumes = linked_list_create();

    if(!volumes) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }
}

const linked_list_t* volume_get_all() {
    return volumes;
}

size_t volume_register_device(storage_device_t* device) {
    // If no MBR is present, create a volume for the entire device
    if(!mbr_probe(device)) {
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
        itoa(1, partition_number, 10);

        strcpy(volume->name + strlen(device->info.name) + 2, partition_number);

        volume->offset = 0;
        volume->size = device->driver->total_size();
        volume->device = device;

        volume->operations = (volume_operations_t*) kmalloc(sizeof(volume_operations_t));

        if(!volume->operations) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        volume->operations->total_size = volume_total_size;
        volume->operations->read = volume_read;
        volume->operations->write = volume_write;

        linked_list_node_t* new_node = linked_list_create_node(volume);

        if(!new_node) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        linked_list_append(volumes, new_node);

        return 1;
    }

    mbr_t* mbr = mbr_read(device);
    size_t registered_volume_count = 0;

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

        volume->offset = partition->lba_start * MBR_SECTION_SIZE;
        volume->size = partition->sectors * MBR_SECTION_SIZE;
        volume->device = device;

        volume->operations = (volume_operations_t*) kmalloc(sizeof(volume_operations_t));

        if(!volume->operations) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        volume->operations->total_size = volume_total_size;
        volume->operations->read = volume_read;
        volume->operations->write = volume_write;

        linked_list_node_t* new_node = linked_list_create_node(volume);

        if(!new_node) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        linked_list_append(volumes, new_node);

        registered_volume_count++;
    }

    return registered_volume_count;
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

static bool volume_find_by_id_compare(void* node_data, void* compare_data) {
    volume_t* volume = (volume_t*) node_data;
    uuid_t* id = (uuid_t*) compare_data;

    return uuid_v4_compare(&volume->id, id) == 0;
}

const volume_t* volume_find_by_id(uuid_t id) {
    linked_list_node_t* node = linked_list_find(volumes, volume_find_by_id_compare, &id);

    if(!node) {
        return NULL;
    }

    return (volume_t*) node->data;
}

static bool volume_find_by_name_compare(void* node_data, void* compare_data) {
    volume_t* volume = (volume_t*) node_data;
    const char* name = (const char*) compare_data;

    return strcmp(volume->name, name) == 0;
}

const volume_t* volume_find_by_name(const char* name) {
    linked_list_node_t* node = linked_list_find(volumes, volume_find_by_name_compare, (void*) name);

    if(!node) {
        return NULL;
    }

    return (volume_t*) node->data;
}

static size_t volume_total_size(volume_t* volume) {
    return volume->size;
}

static size_t volume_read(volume_t* volume, size_t offset, size_t size, char* buffer) {
    if(offset > volume->size) {
        return 0;
    }

    if(offset + size > volume->size) {
        size = volume->size - offset;
    }

    return volume->device->driver->read(volume->offset + offset, size, buffer);
}

static size_t volume_write(volume_t* volume, size_t offset, size_t size, char* buffer) {
    if(offset > volume->size) {
        return 0;
    }

    if(offset + size > volume->size) {
        size = volume->size - offset;
    }

    return volume->device->driver->write(volume->offset * MBR_SECTION_SIZE + offset, size, buffer);
}
