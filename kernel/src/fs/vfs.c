#include <fs/vfs.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>

static vfs_mount_t* vfs_mountpoints[VFS_MAX_MOUNTPOINTS];

static bool vfs_is_valid_absolute_path(char* path);
static vfs_mount_t* vfs_get_mountpoint(char* path);
static int32_t vfs_get_drive_index(char drive);
static vfs_node_t* vfs_findpath_recursive(vfs_node_t* node, char* path);

int32_t vfs_mount(char drive, vfs_mount_t* mountpoint) {
    int32_t index = vfs_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(vfs_mountpoints[index]) {
        return -1;
    }

    if(mountpoint->operations->mount(mountpoint) != 0) {
        return -1;
    }

    vfs_mountpoints[index] = mountpoint;

    return 0;
}

int32_t vfs_unmount(char drive) {
    int32_t index = vfs_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(!vfs_mountpoints[index]) {
        return -1;
    }

    if(vfs_mountpoints[index]->operations->unmount(vfs_mountpoints[index]) != 0) {
        return -1;
    }

    vfs_mountpoints[index] = NULL;

    return 0;
}

static bool vfs_is_valid_absolute_path(char* path) {
    if(!path) {
        return false;
    }

    if(strlen(path) < 3 || strlen(path) > VFS_MAX_PATH_LENGTH) {
        return false;
    }

    if(!isalpha(path[0]) || path[1] != ':' || path[2] != '/') {
        return false;
    }

    return true;
}

static vfs_mount_t* vfs_get_mountpoint(char* path) {
    if(!vfs_is_valid_absolute_path(path)) {
        return NULL;
    }

    int32_t index = vfs_get_drive_index(path[0]);

    if(index == -1) {
        return NULL;
    }

    return vfs_mountpoints[index];
}

static int32_t vfs_get_drive_index(char drive) {
    if(drive >= 'a' && drive <= 'z') {
        return drive - 'a';
    }

    if(drive >= 'A' && drive <= 'Z') {
        return drive - 'A';
    }

    return -1;
}

vfs_node_t* vfs_findpath(char* path) {
    if(!vfs_is_valid_absolute_path(path)) {
        return NULL;
    }

    vfs_mount_t* mountpoint = vfs_get_mountpoint(path);
    char* mountpoint_relative_path = path + 2;

    if(!mountpoint) {
        return NULL;
    }

    char* mountpoint_relative_path_copy = (char*) kmalloc(strlen(mountpoint_relative_path) + 1);

    if(!mountpoint_relative_path_copy) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(mountpoint_relative_path_copy, mountpoint_relative_path);

    if(strcmp(mountpoint_relative_path_copy, "/") == 0) {
        kfree(mountpoint_relative_path_copy);

        return mountpoint->root;
    }

    vfs_node_t* node = vfs_findpath_recursive(mountpoint->root, mountpoint_relative_path_copy + 1);

    kfree(mountpoint_relative_path_copy);

    return node;
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
