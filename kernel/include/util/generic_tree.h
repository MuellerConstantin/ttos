#ifndef _KERNEL_UTIL_GENERIC_TREE_H
#define _KERNEL_UTIL_GENERIC_TREE_H

#include <stdbool.h>
#include <memory/kheap.h>

typedef struct generic_tree_node generic_tree_node_t;
typedef struct generic_tree generic_tree_t;

struct generic_tree_node {
    struct generic_tree_node* parent;
    struct generic_tree_node* children;
    struct generic_tree_node* next;
    void* data;
};

struct generic_tree {
    generic_tree_node_t* root;
};

/**
 * Create a new generic tree.
 * 
 * @return The new generic tree.
 */
static inline generic_tree_t* generic_tree_create() {
    generic_tree_t* tree = (generic_tree_t*) kmalloc(sizeof(generic_tree_t));

    if(!tree) {
        return NULL;
    }

    tree->root = NULL;

    return tree;
}

static inline void _generic_tree_destroy_recursive(generic_tree_node_t* node, bool free_data) {
    if(node->children != NULL) {
        generic_tree_node_t* current = node->children;

        while(current != NULL) {
            generic_tree_node_t* next = current->next;
            _generic_tree_destroy_recursive(current, free_data);
            current = next;
        }
    }

    if(free_data) {
        kfree(node->data);
    }

    kfree(node);
}

/**
 * Destroy the generic tree. It frees the allocated memory for all nodes
 * and the tree metadata. Optionally, it can also free the data.
 * 
 * @param tree The tree to destroy.
 * @param free_data Whether to free the data.
 */
static inline void generic_tree_destroy(generic_tree_t* tree, bool free_data) {
    if(tree->root != NULL) {
        _generic_tree_destroy_recursive(tree->root, free_data);
    }

    kfree(tree);
}

/**
 * Create a new generic tree node.
 * 
 * @param data The data to store in the node.
 * @return The new generic tree node.
 */
static inline generic_tree_node_t* generic_tree_create_node(void* data) {
    generic_tree_node_t* node = (generic_tree_node_t*) kmalloc(sizeof(generic_tree_node_t));

    if(!node) {
        return NULL;
    }

    node->parent = NULL;
    node->children = NULL;
    node->next = NULL;
    node->data = data;

    return node;
}

/**
 * Determine if the tree is empty.
 * 
 * @param tree The tree to check.
 */
static inline bool generic_tree_empty(generic_tree_t* tree) {
    return tree->root == NULL;
}

/**
 * Determine if the node has children.
 * 
 * @param node The node to check.
 */
static inline bool generic_tree_has_children(generic_tree_node_t* node) {
    return node->children != NULL;
}

/**
 * Insert a new node as a child of the given node.
 * 
 * @param parent The parent node.
 * @param node The new node to insert.
 */
static inline void generic_tree_insert(generic_tree_node_t* parent, generic_tree_node_t* node) {
    if(parent->children == NULL) {
        parent->children = node;
    } else {
        generic_tree_node_t* last = parent->children;

        while(last->next != NULL) {
            last = last->next;
        }

        last->next = node;
    }

    node->parent = parent;
}

/**
 * Remove a node from the tree. The node, its data or its children are not freed.
 * 
 * @param tree The tree to remove the node from.
 * @param node The node to remove.
 */
static inline void generic_tree_remove(generic_tree_t* tree, generic_tree_node_t* node) {
    if(node->parent == NULL) {
        return;
    }

    if(node->parent->children == node) {
        node->parent->children = node->next;
    } else {
        generic_tree_node_t* current = node->parent->children;

        while(current->next != node) {
            current = current->next;
        }

        current->next = node->next;
    }

    if(tree->root == node) {
        tree->root = NULL;
    }

    node->parent = NULL;
}

static inline generic_tree_node_t* _generic_tree_find_recursive(generic_tree_node_t* node, bool (*compare)(void* node_data, void* compare_data), void* data) {
    if(compare(node->data, data)) {
        return node;
    }

    if(node->children != NULL) {
        generic_tree_node_t* current = node->children;

        while(current != NULL) {
            generic_tree_node_t* result = _generic_tree_find_recursive(current, compare, data);

            if(result != NULL) {
                return result;
            }

            current = current->next;
        }
    }

    return NULL;
}

/**
 * Find a node in the tree.
 * 
 * @param tree The tree to search.
 * @param compare The comparison function.
 * @param data The data to compare.
 * @return The node if found, NULL otherwise.
 */
static inline generic_tree_node_t* generic_tree_find(generic_tree_t* tree, bool (*compare)(void* node_data, void* compare_data), void* data) {
    if(tree->root == NULL) {
        return NULL;
    }

    return _generic_tree_find_recursive(tree->root, compare, data);
}

static inline void _generic_tree_foreach_recursive(generic_tree_node_t* node, void (*callback)(generic_tree_node_t* node, void* userdata), void* userdata) {
    callback(node, userdata);

    if(node->children != NULL) {
        generic_tree_node_t* current = node->children;

        while(current != NULL) {
            _generic_tree_foreach_recursive(current, callback, userdata);
            current = current->next;
        }
    }
}

/**
 * Iterate over all nodes in the tree.
 * 
 * @param tree The tree to iterate over.
 * @param callback The callback function to call for each node.
 * @param userdata User data to pass to the callback.
 */
static inline void generic_tree_foreach(generic_tree_t* tree, void (*callback)(generic_tree_node_t* node, void* userdata), void* userdata) {
    if(tree->root == NULL) {
        return;
    }

    _generic_tree_foreach_recursive(tree->root, callback, userdata);
}

#endif // _KERNEL_UTIL_GENERIC_TREE_H
