#include <io/vfs.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>
#include <common/linked_list.h>

static linked_list_t* vfs_mountpoints = NULL;

static bool vfs_is_mountpoint(char* path);
static bool vfs_is_mountpoint_compare(char* path, vfs_mountpoint_t* mountpoint);
static vfs_node_t* vfs_findpath_recursive(vfs_node_t* node, char* path);

void vfs_init() {
    vfs_mountpoints = linked_list_init();
}

static bool vfs_is_mountpoint(char* path) {
    int32_t index = linked_list_find(vfs_mountpoints, path, vfs_is_mountpoint_compare);
}

static bool vfs_is_mountpoint_compare(char* path, vfs_mountpoint_t* mountpoint) {
    return strncmp(path, mountpoint->path, strlen(mountpoint->path)) == 0;
}

int32_t vfs_mount(char* path, vfs_node_t* node) {
    if(vfs_is_mountpoint(path)) {
        return -1;
    }

    vfs_mountpoint_t* mountpoint = (vfs_mountpoint_t*) kmalloc(sizeof(vfs_mountpoint_t));

    if(!mountpoint) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    mountpoint->path = (char*) kmalloc(strlen(path) + 1);
    strcpy(mountpoint->path, path);
    mountpoint->node = node;

    linked_list_append(vfs_mountpoints, mountpoint);
}

int32_t vfs_unmount(char* path) {
    int32_t index = linked_list_find(vfs_mountpoints, path, vfs_is_mountpoint_compare);

    if(index == -1) {
        return -1;
    }

    vfs_mountpoint_t* mountpoint = (vfs_mountpoint_t*) linked_list_remove(vfs_mountpoints, index);

    kfree(mountpoint);

    return 0;
}

vfs_mountpoint_t* vfs_get_mountpoint(char* path) {
    int32_t index = linked_list_find(vfs_mountpoints, path, vfs_is_mountpoint_compare);

    if(index == -1) {
        return NULL;
    }

    return (vfs_mountpoint_t *) linked_list_get(vfs_mountpoints, index);
}

char* vfs_get_relative_path(char* path) {
    vfs_mountpoint_t* mountpoint = vfs_get_mountpoint(path);

    if(!mountpoint) {
        return path;
    }

    if(strlen(path) <= strlen(mountpoint->path) + 1) {
        return NULL;
    }

    return path + strlen(mountpoint->path) + 1;
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

vfs_node_t* vfs_findpath(char* path) {
    vfs_mountpoint_t* mountpoint = vfs_get_mountpoint(path);

    if(!mountpoint) {
        return NULL;
    }

    char* relative_path = vfs_get_relative_path(path);

    if(!relative_path) {
        return mountpoint->node;
    }

    return vfs_findpath_recursive(mountpoint->node, relative_path);
}

static vfs_node_t* vfs_findpath_recursive(vfs_node_t* node, char* path) {
    if(!node || !(node->flags & VFS_DIRECTORY)) {
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
