#include <common/linked_list.h>
#include <memory/kheap.h>

linked_list_t* linked_list_init() {
    linked_list_t* list = (linked_list_t*) kmalloc(sizeof(linked_list_t));

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

int32_t linked_list_append(linked_list_t* list, void* data) {
    if(!list) {
        return -1;
    }

    linked_list_node_t* node = (linked_list_node_t*) kmalloc(sizeof(linked_list_node_t));

    node->data = data;
    node->next = NULL;

    if (list->tail) {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    } else {
        list->head = node;
        list->tail = node;
        node->prev = NULL;
    }

    list->size++;

    return 0;
}

int32_t linked_list_prepend(linked_list_t* list, void* data) {
    if(!list) {
        return -1;
    }

    linked_list_node_t* node = (linked_list_node_t*) kmalloc(sizeof(linked_list_node_t));

    node->data = data;
    node->prev = NULL;

    if (list->head) {
        list->head->prev = node;
        node->next = list->head;
        list->head = node;
    } else {
        list->head = node;
        list->tail = node;
        node->next = NULL;
    }

    list->size++;

    return 0;
}

int32_t linked_list_insert(linked_list_t* list, void* data, size_t index) {
    if(!list) {
        return -1;
    }

    if (index >= list->size) {
        linked_list_append(list, data);
    }

    linked_list_node_t* node = (linked_list_node_t*) kmalloc(sizeof(linked_list_node_t));
    node->data = data;

    linked_list_node_t* current_node = list->head;
    size_t current_index = 0;

    while (current_node && current_index < index) {
        current_node = current_node->next;
        current_index++;
    }

    if (current_node->prev) {
        current_node->prev->next = node;
        node->prev = current_node->prev;
    } else {
        list->head = node;
        node->prev = NULL;
    }

    node->next = current_node;
    current_node->prev = node;

    list->size++;

    return 0;
}

void* linked_list_remove(linked_list_t* list, size_t index) {
    if (!list || index >= list->size) {
        return NULL;
    }

    linked_list_node_t* current_node = list->head;
    size_t current_index = 0;

    while (current_node && current_index < index) {
        current_node = current_node->next;
        current_index++;
    }

    if (current_node->prev) {
        current_node->prev->next = current_node->next;
    } else {
        list->head = current_node->next;
    }

    if (current_node->next) {
        current_node->next->prev = current_node->prev;
    } else {
        list->tail = current_node->prev;
    }

    void* data = current_node->data;

    kfree(current_node);
    list->size--;

    return data;
}

void* linked_list_get(linked_list_t* list, size_t index) {
    if (!list || index >= list->size) {
        return NULL;
    }

    linked_list_node_t* current_node = list->head;
    size_t current_index = 0;

    while (current_node && current_index < index) {
        current_node = current_node->next;
        current_index++;
    }

    return current_node->data;
}

size_t linked_list_size(linked_list_t* list) {
    return list->size;
}

int32_t linked_list_clear(linked_list_t* list, bool free_data) {
    if(!list) {
        return -1;
    }

    linked_list_node_t* current_node = list->head;

    while (current_node) {
        linked_list_node_t* next_node = current_node->next;

        if (free_data) {
            kfree(current_node->data);
        }

        kfree(current_node);

        current_node = next_node;
    }
    
    size_t num_cleared = list->size;

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return num_cleared;
}

int32_t linked_list_index_of(linked_list_t* list, void* data) {
    if(!list) {
        return -1;
    }

    linked_list_node_t* current_node = list->head;
    size_t index = 0;

    while (current_node) {
        if (current_node->data == data) {
            return index;
        }

        current_node = current_node->next;
        index++;
    }

    return -1;
}
