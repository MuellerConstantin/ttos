#include <fs/initfs.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <util/string.h>

static int32_t initfs_mount(vfs_filesystem_t* filesystem);
static int32_t initfs_unmount(vfs_filesystem_t* filesystem);

static int32_t initfs_open(vfs_node_t* node);
static int32_t initfs_close(vfs_node_t* node);
static int32_t initfs_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
static int32_t initfs_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
static int32_t initfs_create(vfs_node_t* node, char* name, uint32_t permissions);
static int32_t initfs_unlink(vfs_node_t* node, char* name);

static int32_t initfs_mkdir(vfs_node_t* node, char* name, uint32_t permissions);
static int32_t initfs_rmdir(vfs_node_t* node, char* name);
static vfs_dirent_t* initfs_readdir(vfs_node_t* node, uint32_t index);
static vfs_node_t* initfs_finddir(vfs_node_t* node, char* name);

static int32_t initfs_rename(vfs_node_t* node, char* new_name);

static vfs_node_operations_t initfs_directory_operations = {
    .open = &initfs_open,
    .close = &initfs_close,
    .rename = &initfs_rename,
    .read = NULL,
    .write = NULL,
    .create = NULL,
    .unlink = NULL,
    .mkdir = &initfs_mkdir,
    .rmdir = &initfs_rmdir,
    .readdir = &initfs_readdir,
    .finddir = &initfs_finddir,
};

static vfs_node_operations_t initfs_file_operations = {
    .open = &initfs_open,
    .close = &initfs_close,
    .rename = &initfs_rename,
    .read = &initfs_read,
    .write = &initfs_write,
    .create = &initfs_create,
    .unlink = &initfs_unlink,
    .mkdir = NULL,
    .rmdir = NULL,
    .readdir = NULL,
    .finddir = NULL,
};

bool initfs_probe(volume_t* volume) {
    initfs_header_t initfs_header;

    volume->operations->read(volume, 0, sizeof(initfs_header_t), (char*) &initfs_header);

    return initfs_header.magic == INITFS_HEADER_MAGIC;
}

vfs_filesystem_t* initfs_init(volume_t* volume) {
    if(!initfs_probe(volume)) {
        return NULL;
    }

    vfs_filesystem_t* initfs_mountpoint = (vfs_filesystem_t*) kmalloc(sizeof(vfs_filesystem_t));

    if(!initfs_mountpoint) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    initfs_mountpoint->root = NULL;
    initfs_mountpoint->volume = volume;

    initfs_mountpoint->operations = (vfs_filesystem_operations_t*) kmalloc(sizeof(vfs_filesystem_operations_t));

    if(!initfs_mountpoint->operations) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    initfs_mountpoint->operations->mount = &initfs_mount;
    initfs_mountpoint->operations->unmount = &initfs_unmount;

    return initfs_mountpoint;
}

static int32_t initfs_mount(vfs_filesystem_t* filesystem) {
    vfs_node_t* root = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));

    if(!root) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    root->name = (char*) kmalloc(2);

    if(!root->name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(root->name, "/");

    root->type = VFS_DIRECTORY;
    root->permissions = 0;
    root->uid = 0;
    root->gid = 0;
    root->inode = 0;
    root->length = 0;
    root->link = NULL;
    root->operations = &initfs_directory_operations;
    root->filesystem = filesystem;

    filesystem->root = root;

    return 0;
}

static int32_t initfs_unmount(vfs_filesystem_t* filesystem) {
    kfree(filesystem->root->name);
    kfree(filesystem->root);
    kfree(filesystem);

    return 0;
}

static int32_t initfs_open(vfs_node_t* node) {
    // This file system does not have to load data for directories
    if(node->type == VFS_DIRECTORY) {
        return 0;
    }

    initfs_header_t initfs_header;
    node->filesystem->volume->operations->read(node->filesystem->volume, 0, sizeof(initfs_header_t), &initfs_header);

    if(node->inode >= initfs_header.n_files) {
        return -1;
    }

    initfs_file_header_t* file_header = kmalloc(sizeof(initfs_file_header_t));

    if(!file_header) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    node->filesystem->volume->operations->read(node->filesystem->volume, sizeof(initfs_header_t) + node->inode * sizeof(initfs_file_header_t), sizeof(initfs_file_header_t), file_header);

    if(file_header->magic != INITFS_FILE_HEADER_MAGIC) {
        return -1;
    }

    node->inode_data = (void*) file_header;

    return 0;
}

static int32_t initfs_close(vfs_node_t* node) {
    // This applys only to file nodes
    kfree(node->inode_data);

    return 0;
}

static int32_t initfs_rename(vfs_node_t* node, char* new_name) {
    // Unsupported because the file system is read-only
    return -1;
}

static int32_t initfs_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if(node->inode_data == NULL) {
        return -1;
    }

    initfs_file_header_t* file_header = (initfs_file_header_t*) node->inode_data;

    if(offset > file_header->length) {
        return 0;
    }

    if(offset + size > file_header->length) {
        size = file_header->length - offset;
    }

    node->filesystem->volume->operations->read(node->filesystem->volume, file_header->offset + offset, size, (char*) buffer);

    return size;
}

static int32_t initfs_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    // Unsupported because the file system is read-only
    return -1;
}

static int32_t initfs_create(vfs_node_t* node, char* name, uint32_t permissions) {
    // Unsupported because the file system is read-only
    return -1;
}

static int32_t initfs_unlink(vfs_node_t* node, char* name) {
    // Unsupported because the file system is read-only
    return -1;
}

static int32_t initfs_mkdir(vfs_node_t* node, char* name, uint32_t permissions) {
    // Unsupported because the file system is read-only
    return -1;
}

static int32_t initfs_rmdir(vfs_node_t* node, char* name) {
    // Unsupported because the file system is read-only
    return -1;
}

static vfs_dirent_t* initfs_readdir(vfs_node_t* node, uint32_t index) {
    initfs_header_t initfs_header;
    node->filesystem->volume->operations->read(node->filesystem->volume, 0, sizeof(initfs_header_t), &initfs_header);

    if(index >= initfs_header.n_files) {
        return NULL;
    }

    vfs_dirent_t* dirent = (vfs_dirent_t*) kmalloc(sizeof(vfs_dirent_t));

    if(!dirent) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    initfs_file_header_t file_header;
    node->filesystem->volume->operations->read(node->filesystem->volume, sizeof(initfs_header_t) + index * sizeof(initfs_file_header_t), sizeof(initfs_file_header_t), &file_header);

    if(file_header.magic != INITFS_FILE_HEADER_MAGIC) {
        return NULL;
    }

    strcpy(dirent->name, (const char*) file_header.name);
    dirent->inode = index;

    return dirent;
}

static vfs_node_t* initfs_finddir(vfs_node_t* node, char* name) {
    initfs_header_t initfs_header;
    node->filesystem->volume->operations->read(node->filesystem->volume, 0, sizeof(initfs_header_t), &initfs_header);

    for(size_t index = 0; index < initfs_header.n_files; index++) {
        initfs_file_header_t file_header;
        node->filesystem->volume->operations->read(node->filesystem->volume, sizeof(initfs_header_t) + index * sizeof(initfs_file_header_t), sizeof(initfs_file_header_t), &file_header);

        if(!strcmp(name, file_header.name)) {
            vfs_node_t* new_node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));

            if(!new_node) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            new_node->name = (char*) kmalloc(strlen(file_header.name));

            if(!new_node->name) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            strcpy(new_node->name, (const char*) file_header.name);

            new_node->type = VFS_FILE;
            new_node->permissions = 0;
            new_node->uid = 0;
            new_node->gid = 0;
            new_node->inode = index;
            new_node->length = file_header.length;
            new_node->link = NULL;
            new_node->operations = &initfs_file_operations;
            new_node->filesystem = node->filesystem;

            return new_node;
        }
    }

    return NULL;
}
