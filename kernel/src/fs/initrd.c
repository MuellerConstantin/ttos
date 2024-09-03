#include <fs/initrd.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>
#include <string.h>

static initrd_header_t* initrd_header;
static initrd_file_header_t* initrd_file_headers;

int32_t initrd_mount(mnt_volume_t* volume);
static int32_t initrd_unmount(mnt_volume_t* volume);
static int32_t initrd_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
static vfs_dirent_t* initrd_readdir(vfs_node_t* node, uint32_t index);
static vfs_node_t* initrd_finddir(vfs_node_t* node, char* name);

static mnt_volume_operations_t initrd_volume_operations = {
    .mount = &initrd_mount,
    .unmount = &initrd_unmount
};

static vfs_node_operations_t initrd_directory_operations = {
    .read = NULL,
    .write = NULL,
    .open = NULL,
    .close = NULL,
    .create = NULL,
    .unlink = NULL,
    .mkdir = NULL,
    .rmdir = NULL,
    .rename = NULL,
    .readdir = &initrd_readdir,
    .finddir = &initrd_finddir
};

static vfs_node_operations_t initrd_file_operations = {
    .read = &initrd_read,
    .write = NULL,
    .open = NULL,
    .close = NULL,
    .create = NULL,
    .unlink = NULL,
    .mkdir = NULL,
    .rmdir = NULL,
    .rename = NULL,
    .readdir = NULL,
    .finddir = NULL
};

mnt_volume_t* initrd_init(void* memory_base) {
    initrd_header = (initrd_header_t*) memory_base;

    if(initrd_header->magic != INITRD_HEADER_MAGIC) {
        return -1;
    }

    initrd_file_headers = (initrd_file_header_t*) ((uintptr_t) memory_base + sizeof(initrd_header_t));

    mnt_volume_t* initrd_volume = (mnt_volume_t*) kmalloc(sizeof(mnt_volume_t));

    if(!initrd_volume) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    initrd_volume->operations = &initrd_volume_operations;

    return initrd_volume;
}

int32_t initrd_mount(mnt_volume_t* volume) {
    vfs_node_t* root = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));

    if(!root) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(root->name, "initrd");
    root->type = VFS_DIRECTORY;
    root->permissions = 0;
    root->uid = 0;
    root->gid = 0;
    root->inode = 0;
    root->length = 0;
    root->link = NULL;
    root->operations = &initrd_directory_operations;

    volume->root = root;

    return 0;
}

static int32_t initrd_unmount(mnt_volume_t* mount) {
    kfree(mount->root);
    kfree(mount);

    return 0;
}

static int32_t initrd_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if(node->type & VFS_DIRECTORY) {
        return -1;
    }

    if(node->inode >= initrd_header->n_files) {
        return -1;
    }

    initrd_file_header_t* file_header = &initrd_file_headers[node->inode];

    if(offset > file_header->length) {
        return 0;
    }

    if(offset + size > file_header->length) {
        size = file_header->length - offset;
    }

    memcpy(buffer, (void*) (file_header->offset + (uintptr_t) initrd_header + offset), size);

    return size;
}

static vfs_dirent_t* initrd_readdir(vfs_node_t* node, uint32_t index) {
    if(index >= initrd_header->n_files) {
        return NULL;
    }

    vfs_dirent_t* dirent = (vfs_dirent_t*) kmalloc(sizeof(vfs_dirent_t));

    if(!dirent) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    initrd_file_header_t* file_header = &initrd_file_headers[index];

    strcpy(dirent->name, (const char*) file_header->name);
    dirent->inode = index;

    return dirent;
}

static vfs_node_t* initrd_finddir(vfs_node_t* node, char* name) {
    for(size_t index = 0; index < initrd_header->n_files; index++) {
        initrd_file_header_t* file_header = &initrd_file_headers[index];

        if(!strcmp(name, file_header->name)) {
            vfs_node_t* new_node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));

            if(!new_node) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            strcpy(new_node->name, (const char*) file_header->name);
            new_node->type = VFS_FILE;
            new_node->permissions = 0;
            new_node->uid = 0;
            new_node->gid = 0;
            new_node->inode = index;
            new_node->length = file_header->length;
            new_node->link = NULL;
            new_node->operations = &initrd_file_operations;

            return new_node;
        }
    }

    return NULL;
}
