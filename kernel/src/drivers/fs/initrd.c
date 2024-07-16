#include <drivers/fs/initrd.h>
#include <sys/kpanic.h>
#include <string.h>

static initrd_header_t* initrd_header;
static initrd_file_header_t* initrd_file_headers;
static vfs_node_t* initrd_root;
static vfs_node_t* initrd_root_nodes;
static size_t initrd_num_root_nodes;

static int32_t initrd_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
static vfs_dirent_t* initrd_readdir(vfs_node_t* node, uint32_t index);
static vfs_node_t* initrd_finddir(vfs_node_t* node, char* name);

int32_t initrd_init(void* memory_base) {
    initrd_header = (initrd_header_t*) memory_base;

    if(initrd_header->magic != INITRD_HEADER_MAGIC) {
        return -1;
    }

    initrd_root = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));

    if(!initrd_root) {
        return -1;
    }

    strcpy(initrd_root->name, "initrd");
    initrd_root->flags = VFS_DIRECTORY | VFS_MOUNTPOINT;
    initrd_root->permissions = 0;
    initrd_root->uid = 0;
    initrd_root->gid = 0;
    initrd_root->inode = 0;
    initrd_root->length = 0;
    initrd_root->impl = 0;
    initrd_root->ptr = NULL;
    initrd_root->read = NULL;
    initrd_root->write = NULL;
    initrd_root->open = NULL;
    initrd_root->close = NULL;
    initrd_root->readdir = &initrd_readdir;
    initrd_root->finddir = &initrd_finddir;

    initrd_file_headers = (initrd_file_header_t*) ((uintptr_t) memory_base + sizeof(initrd_header_t));
    initrd_num_root_nodes = initrd_header->n_files;
    
    if(initrd_num_root_nodes > 0) {
        initrd_root_nodes = (vfs_node_t*) kmalloc(sizeof(vfs_node_t) * initrd_num_root_nodes);

        if(!initrd_root_nodes) {
            kfree(initrd_root);
            return -1;
        }

        for(size_t index = 0; index < initrd_num_root_nodes; index++) {
            if(initrd_file_headers[index].magic != INITRD_FILE_HEADER_MAGIC) {
                return -1;
            }

            initrd_file_headers[index].offset += (uint32_t) memory_base;

            strcpy(initrd_root_nodes[index].name, (const char*) initrd_file_headers[index].name);
            initrd_root_nodes[index].flags = VFS_FILE;
            initrd_root_nodes[index].permissions = 0;
            initrd_root_nodes[index].uid = 0;
            initrd_root_nodes[index].gid = 0;
            initrd_root_nodes[index].inode = index;
            initrd_root_nodes[index].length = initrd_file_headers[index].length;
            initrd_root_nodes[index].impl = 0;
            initrd_root_nodes[index].ptr = NULL;
            initrd_root_nodes[index].read = &initrd_read;
            initrd_root_nodes[index].write = NULL;
            initrd_root_nodes[index].open = NULL;
            initrd_root_nodes[index].close = NULL;
            initrd_root_nodes[index].readdir = NULL;
            initrd_root_nodes[index].finddir = NULL;
        }
    }

    vfs_mount("/mnt/initrd", initrd_root);

    return 0;
}

static int32_t initrd_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if(node->flags & VFS_DIRECTORY) {
        return -1;
    }

    initrd_file_header_t* file_header = &initrd_file_headers[node->inode];

    if(offset > file_header->length) {
        return 0;
    }

    if(offset + size > file_header->length) {
        size = file_header->length - offset;
    }

    memcpy(buffer, (void*) (file_header->offset + offset), size);

    return size;
}

static vfs_dirent_t* initrd_readdir(vfs_node_t* node, uint32_t index) {
    if(index >= initrd_num_root_nodes) {
        return NULL;
    }

    vfs_dirent_t* dirent = (vfs_dirent_t*) kmalloc(sizeof(vfs_dirent_t));

    if(!dirent) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(dirent->name, initrd_root_nodes[index].name);
    dirent->inode = initrd_root_nodes[index].inode;

    return dirent;
}

static vfs_node_t* initrd_finddir(vfs_node_t* node, char* name) {
    for(size_t index = 0; index < initrd_num_root_nodes; index++) {
        if(!strcmp(name, initrd_root_nodes[index].name)) {
            return &initrd_root_nodes[index];
        }
    }

    return NULL;
}
