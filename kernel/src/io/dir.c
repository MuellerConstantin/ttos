#include <io/dir.h>
#include <fs/mount.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

dir_descriptor_t dir_descriptors[MAX_DIR_DESCRIPTORS];

static int32_t dir_get_free_descriptor();

int32_t dir_open(char* path) {
    if(!vfs_is_abs_path(path)) {
        return -1;
    }

    mnt_mountpoint_t* mountpoint = mnt_get_mountpoint(path);

    if(!mountpoint) {
        return -1;
    }

    char* relative_path = path + 3;

    vfs_node_t* node = vfs_findpath(mountpoint->root, relative_path);

    if(!node) {
        return -1;
    }

    if(node->type != VFS_DIRECTORY) {
        kfree(node);
        return -1;
    }

    int32_t dd = dir_get_free_descriptor();

    if(dd < 0) {
        kfree(node);
        return -1;
    }

    // Copy the root node to avoid freeing the root node when closing the directory
    if(node == mountpoint->root) {
        vfs_node_t* root_copy = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));

        if(!root_copy) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        memcpy(root_copy, node, sizeof(vfs_node_t));

        node = root_copy;
    }

    dir_descriptors[dd].node = node;
    dir_descriptors[dd].index = 0;

    return dd;
}

int32_t dir_close(int32_t dd) {
    if(dd < 0 || dd >= MAX_DIR_DESCRIPTORS) {
        return -1;
    }

    if(!dir_descriptors[dd].node) {
        return -1;
    }

    kfree(dir_descriptors[dd].node);
    dir_descriptors[dd].node = NULL;

    return 0;
}

const dir_dirent_t* dir_read(int32_t dd) {
    if(dd < 0 || dd >= MAX_DIR_DESCRIPTORS) {
        return NULL;
    }

    if(!dir_descriptors[dd].node) {
        return NULL;
    }

    vfs_dirent_t* dirent = vfs_readdir(dir_descriptors[dd].node, dir_descriptors[dd].index);

    if(!dirent) {
        return NULL;
    }

    dir_dirent_t* dir_dirent = kmalloc(sizeof(dir_dirent_t));

    if(!dir_dirent) {
        kfree(dirent);
        return NULL;
    }

    strcpy(dir_dirent->name, dirent->name);
    dir_dirent->inode = dirent->inode;

    kfree(dirent);

    dir_descriptors[dd].index++;

    return dir_dirent;
}

static int32_t dir_get_free_descriptor() {
    for(size_t index = 0; index < MAX_DIR_DESCRIPTORS; index++) {
        if(!dir_descriptors[index].node) {
            return index;
        }
    }

    return -1;
}
