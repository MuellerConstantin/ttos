#include <fs/vfs.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>

static vfs_node_t* vfs_findpath_recursive(vfs_node_t* node, char* path);

int32_t vfs_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if (node->operations->read != NULL) {
        return node->operations->read(node, offset, size, buffer);
    }

    return -1;
}

int32_t vfs_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if (node->operations->write != NULL) {
        return node->operations->write(node, offset, size, buffer);
    }

    return -1;
}

int32_t vfs_open(vfs_node_t* node) {
    if (node->operations->open != NULL) {
        return node->operations->open(node);
    }

    return -1;
}

int32_t vfs_close(vfs_node_t* node) {
    if (node->operations->close != NULL) {
        return node->operations->close(node);
    }

    return -1;
}

vfs_dirent_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (node->operations->readdir != NULL && node->type & VFS_DIRECTORY) {
        return node->operations->readdir(node, index);
    }

    return NULL;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, char* name) {
    if (node->operations->finddir != NULL && node->type & VFS_DIRECTORY) {
        return node->operations->finddir(node, name);
    }

    return NULL;
}

vfs_node_t* vfs_findpath(vfs_node_t* node, char* path) {
    char* path_copy = (char*) kmalloc(strlen(path) + 1);

    if(!path_copy) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(path_copy, path);

    vfs_node_t* found_node = vfs_findpath_recursive(node, path_copy);

    kfree(path_copy);

    return found_node;
}

static vfs_node_t* vfs_findpath_recursive(vfs_node_t* node, char* path) {
    if(!node || !(node->type & VFS_DIRECTORY)) {
        return NULL;
    }

    char* token = strsep(&path, "/");

    if(!token) {
        return node;
    }

    vfs_node_t* next_node = vfs_finddir(node, token);

    if(!next_node) {
        return NULL;
    }

    if(!path) {
        return next_node;
    }

    return vfs_findpath_recursive(next_node, path);
}
