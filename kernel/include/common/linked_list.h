#ifndef _KERNEL_COMMON_LINKED_LIST_H
#define _KERNEL_COMMON_LINKED_LIST_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define linked_list_foreach(list, node) for(linked_list_node_t* node = list->head; node != NULL; node = node->next)

struct linked_list_node {
    struct linked_list_node* next;
    struct linked_list_node* prev;
    void* data;
};

typedef struct linked_list_node linked_list_node_t;

struct linked_list {
    linked_list_node_t* head;
    linked_list_node_t* tail;
    size_t size;
};

typedef struct linked_list linked_list_t;

/**
 * Initialize a new linked list.
 * 
 * @return A pointer to the new linked list.
 */
linked_list_t* linked_list_init();

/**
 * Append a new node to the end of the list.
 * 
 * @param list The list to append to.
 * @param data The data to store in the new node.
 * @return 0 if the node was appended successfully, -1 if the list is NULL.
 */
int32_t linked_list_append(linked_list_t* list, void* data);

/**
 * Prepend a new node to the beginning of the list.
 * 
 * @param list The list to prepend to.
 * @param data The data to store in the new node.
 * @return 0 if the node was prepended successfully, -1 if the list is NULL.
 */
int32_t linked_list_prepend(linked_list_t* list, void* data);

/**
 * Insert a new node at the given index in the list.
 * 
 * @param list The list to insert into.
 * @param data The data to store in the new node.
 * @param index The index to insert the new node at.
 * @return 0 if the node was inserted successfully, -1 if the list is NULL.
 */
int32_t linked_list_insert(linked_list_t* list, void* data, size_t index);

/**
 * Remove the node at the given index from the list.
 * 
 * @param list The list to remove from.
 * @param index The index of the node to remove.
 * @return The data stored in the removed node or NULL if the list is NULL.
 */
void* linked_list_remove(linked_list_t* list, size_t index);

/**
 * Get the data stored in the node at the given index.
 * 
 * @param list The list to get the data from.
 * @param index The index of the node to get the data from.
 * @return The data stored in the node.
 */
void* linked_list_get(linked_list_t* list, size_t index);

/**
 * Get the size of the list.
 * 
 * @param list The list to get the size of.
 * @return The size of the list.
 */
size_t linked_list_size(linked_list_t* list);

/**
 * Clear the list, optionally freeing the data stored in the nodes.
 * 
 * @param list The list to clear.
 * @param free_data Whether to free the data stored in the nodes.
 * @return The number of nodes cleared or -1 if the list is NULL.
 */
int32_t linked_list_clear(linked_list_t* list, bool free_data);

/**
 * Find the node in the list that contains the given data.
 * 
 * @param list The list to search.
 * @param data The data to search for.
 * @return The index of the node containing the data, or -1 if the data was not found.
 */
int32_t linked_list_index_of(linked_list_t* list, void* data);

#endif // _KERNEL_COMMON_LINKED_LIST_H
