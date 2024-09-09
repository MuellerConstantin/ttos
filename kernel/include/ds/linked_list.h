#ifndef _KERNEL_DS_LINKED_LIST_H
#define _KERNEL_DS_LINKED_LIST_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory/kheap.h>

typedef struct linked_list_node linked_list_node_t;
typedef struct linked_list linked_list_t;

struct linked_list_node {
    linked_list_node_t* prev;
    linked_list_node_t* next;
    void* data;
};

struct linked_list {
    linked_list_node_t* head;
    linked_list_node_t* tail;
};

#define linked_list_foreach(list, node) \
    for(linked_list_node_t* node = list->head; node != NULL; node = node->next)

/**
 * Create a new linked list.
 * 
 * @return The new linked list.
 */
static inline linked_list_t* linked_list_create() {
    linked_list_t* list = (linked_list_t*) kmalloc(sizeof(linked_list_t));

    if(!list) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;

    return list;
}

/**
 * Create a new linked list node.
 * 
 * @return The new linked list node.
 */
static inline linked_list_node_t* linked_list_create_node(void* data) {
    linked_list_node_t* node = kmalloc(sizeof(linked_list_node_t));

    if(!node) {
        return NULL;
    }

    node->prev = NULL;
    node->next = NULL;
    node->data = data;

    return node;
}

/**
 * Check if the list is empty.
 * 
 * @param list The list to check.
 * @return True if the list is empty, false otherwise.
 */
static inline bool linked_list_empty(linked_list_t *list) {
    return list->head == NULL;
}

/**
 * Insert a new node after the given node.
 * 
 * @param head The node to insert after.
 * @param node The new node to insert.
 */
static inline void linked_list_append(linked_list_t *list, linked_list_node_t* node) {
    if(linked_list_empty(list)) {
        list->head = node;
        list->tail = node;
        node->prev = NULL;
        node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
}

/**
 * Insert a new node before the given node.
 * 
 * @param head The node to insert before.
 * @param node The new node to insert.
 */
static inline void linked_list_prepend(linked_list_t* list, linked_list_node_t* node) {
    if(linked_list_empty(list)) {
        list->head = node;
        list->tail = node;
        node->prev = NULL;
        node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
}

/**
 * Remove a node from the list. The node or its data is not freed.
 * 
 * @param list The list to remove the node from.
 * @param node The node to remove.
 */
static inline void linked_list_remove(linked_list_t* list, linked_list_node_t* node) {
    if(list->head == node) {
        list->head = node->next;
    }

    if(list->tail == node) {
        list->tail = node->prev;
    }

    if(node->prev) {
        node->prev->next = node->next;
    }

    if(node->next) {
        node->next->prev = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

/**
 * Determine the size of the list.
 * 
 * @param head The head of the list.
 * @return The size of the list.
 */
static inline size_t linked_list_size(linked_list_t* list) {
    size_t size = 0;

    linked_list_foreach(list, node) {
        size++;
    }

    return size;
}

/**
 * Get the node at the given index.
 * 
 * @param head The head of the list.
 * @param index The index of the node to get.
 * @return The node at the given index.
 */
static inline linked_list_node_t* linked_list_get(linked_list_t *list, size_t index) {
    size_t current_index = 0;

    linked_list_foreach(list, node) {
        if (current_index == index) {
            return node;
        }

        current_index++;
    }

    return NULL;
}

/**
 * Find a node in the list.
 * 
 * @param list The list to search.
 * @param compare The comparison function.
 * @param data The data to compare.
 * @return The node if found, NULL otherwise.
 */
static inline linked_list_node_t* linked_list_find(linked_list_t* list, bool (*compare)(void*, void*), void* data) {
    linked_list_foreach(list, node) {
        if(compare(node->data, data)) {
            return node;
        }
    }

    return NULL;
}

#endif // _KERNEL_DS_LINKED_LIST_H
