#include <io/vfs.h>

static vfs_node_t* vfs_root = NULL;

void vfs_root_mount(vfs_node_t* node) {
    vfs_root = node;
}

vfs_node_t* vfs_get_root() {
    return vfs_root;
}

int32_t vfs_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if (node->read != NULL) {
        return node->read(node, offset, size, buffer);
    }

    return -1;
}

int32_t vfs_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if (node->write != NULL) {
        return node->write(node, offset, size, buffer);
    }

    return -1;
}

int32_t vfs_open(vfs_node_t* node) {
    if (node->open != NULL) {
        return node->open(node);
    }

    return -1;
}

int32_t vfs_close(vfs_node_t* node) {
    if (node->close != NULL) {
        return node->close(node);
    }

    return -1;
}

vfs_dirent_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (node->readdir != NULL && node->flags & VFS_DIRECTORY) {
        return node->readdir(node, index);
    }

    return NULL;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, char* name) {
    if (node->finddir != NULL && node->flags & VFS_DIRECTORY) {
        return node->finddir(node, name);
    }

    return NULL;
}
