#include <fs/vfs.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

static vfs_node_t* vfs_findpath_recursive(vfs_node_t* node, char* path);

bool vfs_is_abs_path(char* path) {
    if(strlen(path) < 3) {
        return false;
    }

    if((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) {
        if(path[1] == ':' && path[2] == '/') {
            return true;
        }
    }

    return false;
}

bool vfs_is_file(vfs_node_t* node) {
    return node && node->type == VFS_FILE;
}

bool vfs_is_dir(vfs_node_t* node) {
    return node && node->type == VFS_DIRECTORY;
}

int32_t vfs_open(vfs_node_t* node) {
    if (node && node->operations->open != NULL) {
        return node->operations->open(node);
    }

    return -1;
}

int32_t vfs_close(vfs_node_t* node) {
    if (node && node->operations->close != NULL) {
        return node->operations->close(node);
    }

    return -1;
}

int32_t vfs_rename(vfs_node_t* node, char* new_name) {
    if (node && node->operations->rename != NULL) {
        return node->operations->rename(node, new_name);
    }

    return -1;
}

int32_t vfs_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if (node && node->operations->read != NULL && node->type == VFS_FILE) {
        return node->operations->read(node, offset, size, buffer);
    }

    return -1;
}

int32_t vfs_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if (node && node->operations->write != NULL && node->type == VFS_FILE) {
        return node->operations->write(node, offset, size, buffer);
    }

    return -1;
}

int32_t vfs_create(vfs_node_t* node, char* name, uint32_t permissions) {
    if (node && node->operations->create != NULL && node->type == VFS_DIRECTORY) {
        return node->operations->create(node, name, permissions);
    }

    return -1;
}

int32_t vfs_unlink(vfs_node_t* node, char* name) {
    if (node && node->operations->unlink != NULL && node->type == VFS_DIRECTORY) {
        return node->operations->unlink(node, name);
    }

    return -1;
}

int32_t vfs_mkdir(vfs_node_t* node, char* name, uint32_t permissions) {
    if (node && node->operations->mkdir != NULL && node->type == VFS_DIRECTORY) {
        return node->operations->mkdir(node, name, permissions);
    }

    return -1;
}

int32_t vfs_rmdir(vfs_node_t* node, char* name) {
    if (node && node->operations->rmdir != NULL && node->type == VFS_DIRECTORY) {
        return node->operations->rmdir(node, name);
    }

    return -1;
}

vfs_dirent_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (node && node->operations->readdir != NULL && node->type == VFS_DIRECTORY) {
        return node->operations->readdir(node, index);
    }

    return NULL;
}

vfs_node_t* vfs_finddir(vfs_node_t* node, char* name) {
    if (node && node->operations->finddir != NULL && node->type == VFS_DIRECTORY) {
        return node->operations->finddir(node, name);
    }

    return NULL;
}

vfs_node_t* vfs_findpath(vfs_node_t* node, char* path) {
    if(!node || !(node->type == VFS_DIRECTORY) || node->operations->finddir == NULL) {
        return NULL;
    }

    if(strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
        return node;
    }

    char* path_copy = (char*) kmalloc(strlen(path) + 1);

    if(!path_copy) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(path_copy, path);

    if(path_copy[0] == '/') {
        path_copy++;
    }

    vfs_node_t* found_node = vfs_findpath_recursive(node, path_copy);

    kfree(path_copy);

    return found_node;
}

static vfs_node_t* vfs_findpath_recursive(vfs_node_t* node, char* path) {
    if(!node || !(node->type == VFS_DIRECTORY) || node->operations->finddir == NULL) {
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
