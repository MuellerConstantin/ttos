#ifndef _KERNEL_DS_CIRCULAR_LINKED_LIST_H
#define _KERNEL_DS_CIRCULAR_LINKED_LIST_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct circular_linked_list_node circular_linked_list_node_t;

struct circular_linked_list_node {
    circular_linked_list_node_t* next;
    circular_linked_list_node_t* prev;
    void* data;
};

#define circular_linked_list_foreach(head, node) \
    for (circular_linked_list_node_t* node = head; node != head->prev; node = node->next)

/**
 * Initialize a linked list.
 * 
 * @param list The head of the list to initialize.
 */
static inline void circular_linked_list_init(circular_linked_list_node_t *head) {
	head->next = head;
	head->prev = head;
}

/**
 * Insert a new node after the given node.
 * 
 * @param new The new node to insert.
 * @param head The node to insert after.
 */
static inline void circular_linked_list_append(circular_linked_list_node_t *new_node, circular_linked_list_node_t* head) {
    new_node->next = head->next;
    new_node->prev = head;
    head->next = new_node;
    new_node->next->prev = new_node;
}

/**
 * Insert a new node before the given node.
 * 
 * @param new The new node to insert.
 * @param head The node to insert before.
 */
static inline void circular_linked_list_prepend(circular_linked_list_node_t *new_node, circular_linked_list_node_t* head) {
    new_node->next = head;
    new_node->prev = head->prev;
    head->prev = new_node;
    new_node->prev->next = new_node;
}

/**
 * Remove a node from the list.
 * 
 * @param entry The node to remove.
 * @return The node that was removed.
 */
static inline void circular_linked_list_del(circular_linked_list_node_t *entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

/**
 * Check if the list is empty.
 * 
 * @param head The head of the list.
 * @return Whether the list is empty.
 */
static inline bool circular_linked_list_empty(circular_linked_list_node_t *head) {
    return head->next == head;
}

/**
 * Determine the size of the list.
 * 
 * @param head The head of the list.
 * @return The size of the list.
 */
static inline size_t circular_linked_list_size(circular_linked_list_node_t *head) {
    size_t size = 0;

    circular_linked_list_foreach(head, node) {
        size++;
    }

    return size;
}

static inline circular_linked_list_node_t* circular_linked_list_get(circular_linked_list_node_t *head, size_t index) {
    size_t current_index = 0;

    circular_linked_list_foreach(head, node) {
        if (current_index == index) {
            return node;
        }

        current_index++;
    }

    return NULL;
}

#endif // _KERNEL_DS_CIRCULAR_LINKED_LIST_H
