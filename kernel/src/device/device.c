#include <device/device.h>
#include <sys/kpanic.h>

generic_tree_t* device_tree;

static struct _device_find_all_by_type_callback_payload {
    linked_list_t* devices;
    uint16_t type;
};

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

    generate_uuid_v4(&root_device->id);
    strcpy(root_device->name, "Computer");
    root_device->type = DEVICE_TYPE_RESERVED;
    root_device->bus.type = DEVICE_BUS_TYPE_RESERVED;
    root_device->bus.data = NULL;

    generic_tree_node_t* root_node = generic_tree_create_node(root_device);

    if(!root_node) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_tree->root = root_node;
}

generic_tree_t* device_get_all() {
    return device_tree;
}

static bool _device_register_compare(void* node_data, void* compare_data) {
    device_t* node_device = (device_t*) node_data;
    device_t* compare_device = (device_t*) compare_data;

    return node_device == compare_device;
}

void device_register(device_t* parent, device_t* device) {
    generic_tree_node_t* parent_node = NULL;

    if(!parent) {
        parent_node = device_tree->root;
    } else {
        parent_node = generic_tree_find(device_tree, _device_register_compare, parent);
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

static bool _device_unregister_compare(void* node_data, void* compare_data) {
    device_t* node_device = (device_t*) node_data;
    device_t* compare_device = (device_t*) compare_data;

    return node_device == compare_device;
}

void device_unregister(device_t* device) {
    generic_tree_node_t* node = generic_tree_find(device_tree, _device_unregister_compare, device);

    if(!node) {
        KPANIC(KPANIC_DEVICE_NOT_FOUND_CODE, KPANIC_DEVICE_NOT_FOUND_MESSAGE, NULL);
    }

    generic_tree_remove(device_tree, node);
}

static bool _device_find_by_type_compare(void* node_data, void* compare_data) {
    device_t* node_device = (device_t*) node_data;
    uint16_t* compare_type = (uint16_t*) compare_data;

    return node_device->type == *compare_type;
}

device_t* device_find_by_type(uint16_t type) {
    generic_tree_node_t* node = generic_tree_find(device_tree, _device_find_by_type_compare, &type);

    if(!node) {
        return NULL;
    }

    return (device_t*) node->data;
}

static void _device_find_all_by_type_callback(generic_tree_node_t* node, void* data) {
    struct _device_find_all_by_type_callback_payload* payload = (struct _device_find_all_by_type_callback_payload*) data;
    device_t* device = (device_t*) node->data;

    if(device->type == payload->type) {
        linked_list_node_t* devices_node = (linked_list_node_t*) kmalloc(sizeof(linked_list_node_t));

        if(!devices_node) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        devices_node->data = device;

        linked_list_append(payload->devices, devices_node);
    }
}

linked_list_t* device_find_all_by_type(uint16_t type) {
    linked_list_t* devices = linked_list_create();

    if(!devices) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    struct _device_find_all_by_type_callback_payload payload = {
        .devices = devices,
        .type = type
    };

    generic_tree_foreach(device_tree, _device_find_all_by_type_callback, &payload);

    return devices;
}

static bool _device_find_by_id_compare(void* node_data, void* compare_data) {
    device_t* node_device = (device_t*) node_data;
    uuid_t* id = (uuid_t*) compare_data;

    return uuid_v4_compare(&node_device->id, id) == 0;
}

device_t* device_find_by_id(uuid_t id) {
    generic_tree_node_t* node = generic_tree_find(device_tree, _device_find_by_id_compare, &id);

    if(!node) {
        return NULL;
    }

    return (device_t*) node->data;
}
