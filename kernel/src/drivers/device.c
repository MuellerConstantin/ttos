#include <drivers/device.h>
#include <sys/kpanic.h>

generic_tree_t* device_tree;

void device_init() {
    device_tree = generic_tree_create();

    if(!device_tree) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_t* root_device = (device_t*) kmalloc(sizeof(device_t));

    if(!root_device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    root_device->name = (char*) kmalloc(9);

    if(!root_device->name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(root_device->name, "Computer");
    root_device->device_type = DEVICE_TYPE_RESERVED;
    root_device->bus_type = DEVICE_BUS_TYPE_RESERVED;
    root_device->bus_data = NULL;

    generic_tree_node_t* root_node = generic_tree_create_node(root_device);

    if(!root_node) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_tree->root = root_node;
}

generic_tree_t* device_get_tree() {
    return device_tree;
}

static bool _device_register_find_compare(void* node_data, void* compare_data) {
    device_t* node_device = (device_t*) node_data;
    device_t* compare_device = (device_t*) compare_data;

    return node_device == compare_device;
}

static bool _device_unregister_find_compare(void* node_data, void* compare_data) {
    device_t* node_device = (device_t*) node_data;
    device_t* compare_device = (device_t*) compare_data;

    return node_device == compare_device;
}

void device_register(device_t* parent, device_t* device) {
    generic_tree_node_t* parent_node = NULL;

    if(!parent) {
        parent_node = device_tree->root;
    } else {
        parent_node = generic_tree_find(device_tree, _device_register_find_compare, parent);
    }

    if(!parent_node) {
        KPANIC(KPANIC_DEVICE_NOT_FOUND_CODE, KPANIC_DEVICE_NOT_FOUND_MESSAGE, NULL);
    }

    generic_tree_node_t* new_node = generic_tree_create_node(device);

    if(!new_node) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    generic_tree_insert(parent_node, new_node);
}

void device_unregister(device_t* device) {
    generic_tree_node_t* node = generic_tree_find(device_tree, _device_unregister_find_compare, device);

    if(!node) {
        KPANIC(KPANIC_DEVICE_NOT_FOUND_CODE, KPANIC_DEVICE_NOT_FOUND_MESSAGE, NULL);
    }

    generic_tree_remove(device_tree, node);
}
