#include <device/volume.h>
#include <sys/kpanic.h>

linked_list_t* volumes;

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
    return 0;
}

void volume_unregister_device(storage_device_t* device) {

}

int32_t volume_mount(char drive, volume_t* volume) {
    return -1;
}

int32_t volume_unmount(volume_t* volume) {
    return -1;
}

volume_t* volume_find_by_id(uuid_t id) {
    return NULL;
}
