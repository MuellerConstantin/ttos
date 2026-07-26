#include <fs/ext2.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <util/string.h>

/**
 * In-memory state of a mounted ext2 file system. Stored in vfs_filesystem_t::fs_data. Holds the
 * cached superblock together with the values derived from it that are needed on every access.
 */
typedef struct ext2_fs {
    ext2_superblock_t superblock;   // Cached copy of the superblock
    uint32_t block_size;            // Block size in bytes (1024 << s_log_block_size)
    uint32_t inode_size;            // Size of an on-disk inode (with rev-0 fallback)
    uint32_t inodes_per_group;      // Number of inodes per block group
    uint32_t blocks_per_group;      // Number of blocks per block group
    uint32_t bgd_table_block;       // Block number of the block group descriptor table
    uint32_t num_block_groups;      // Number of block groups
} ext2_fs_t;

// File system lifecycle

static int32_t ext2_mount(vfs_filesystem_t* filesystem);
static int32_t ext2_unmount(vfs_filesystem_t* filesystem);

// Node operations

static int32_t ext2_open(vfs_node_t* node);
static int32_t ext2_close(vfs_node_t* node);
static int32_t ext2_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
static int32_t ext2_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
static int32_t ext2_create(vfs_node_t* node, char* name, uint32_t permissions);
static int32_t ext2_unlink(vfs_node_t* node, char* name);
static int32_t ext2_mkdir(vfs_node_t* node, char* name, uint32_t permissions);
static int32_t ext2_rmdir(vfs_node_t* node, char* name);
static vfs_dirent_t* ext2_readdir(vfs_node_t* node, uint32_t index);
static vfs_node_t* ext2_finddir(vfs_node_t* node, char* name);
static int32_t ext2_rename(vfs_node_t* node, char* new_name);

// Internal helpers

static size_t ext2_read_block(vfs_filesystem_t* filesystem, uint32_t block, void* buffer);
static int32_t ext2_read_bgd(vfs_filesystem_t* filesystem, uint32_t group, ext2_block_group_descriptor_t* out);
static int32_t ext2_read_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, ext2_inode_t* out);
static uint32_t ext2_inode_block(vfs_filesystem_t* filesystem, ext2_inode_t* inode, uint32_t index);
static vfs_node_t* ext2_build_node(vfs_filesystem_t* filesystem, uint32_t inode_no, const char* name, ext2_inode_t* inode);

static vfs_node_operations_t ext2_directory_operations = {
    .open = &ext2_open,
    .close = &ext2_close,
    .rename = &ext2_rename,
    .read = NULL,
    .write = NULL,
    .create = NULL,
    .unlink = NULL,
    .mkdir = &ext2_mkdir,
    .rmdir = &ext2_rmdir,
    .readdir = &ext2_readdir,
    .finddir = &ext2_finddir,
};

static vfs_node_operations_t ext2_file_operations = {
    .open = &ext2_open,
    .close = &ext2_close,
    .rename = &ext2_rename,
    .read = &ext2_read,
    .write = &ext2_write,
    .create = &ext2_create,
    .unlink = &ext2_unlink,
    .mkdir = NULL,
    .rmdir = NULL,
    .readdir = NULL,
    .finddir = NULL,
};

bool ext2_probe(volume_t* volume) {
    ext2_superblock_t superblock;

    volume->operations->read(volume, EXT2_SUPERBLOCK_OFFSET, sizeof(ext2_superblock_t), (char*) &superblock);

    return superblock.s_magic == EXT2_SUPER_MAGIC;
}

vfs_filesystem_t* ext2_init(volume_t* volume) {
    if(!ext2_probe(volume)) {
        return NULL;
    }

    vfs_filesystem_t* ext2_mountpoint = (vfs_filesystem_t*) kmalloc(sizeof(vfs_filesystem_t));

    if(!ext2_mountpoint) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ext2_mountpoint->root = NULL;
    ext2_mountpoint->volume = volume;
    ext2_mountpoint->fs_data = NULL;

    ext2_mountpoint->operations = (vfs_filesystem_operations_t*) kmalloc(sizeof(vfs_filesystem_operations_t));

    if(!ext2_mountpoint->operations) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ext2_mountpoint->operations->mount = &ext2_mount;
    ext2_mountpoint->operations->unmount = &ext2_unmount;

    return ext2_mountpoint;
}

static int32_t ext2_mount(vfs_filesystem_t* filesystem) {
    ext2_fs_t* data = (ext2_fs_t*) kmalloc(sizeof(ext2_fs_t));

    if(!data) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    filesystem->volume->operations->read(filesystem->volume, EXT2_SUPERBLOCK_OFFSET, sizeof(ext2_superblock_t), (char*) &data->superblock);

    if(data->superblock.s_magic != EXT2_SUPER_MAGIC) {
        kfree(data);
        return -1;
    }

    // Derive the layout parameters used on every access. The inode size and first inode are only
    // stored in the superblock for dynamic revisions; older revisions use fixed values.
    data->block_size = 1024 << data->superblock.s_log_block_size;
    data->blocks_per_group = data->superblock.s_blocks_per_group;
    data->inodes_per_group = data->superblock.s_inodes_per_group;
    data->inode_size = (data->superblock.s_rev_level >= 1) ? data->superblock.s_inode_size : 128;
    data->bgd_table_block = data->superblock.s_first_data_block + 1;
    data->num_block_groups = (data->superblock.s_blocks_count + data->blocks_per_group - 1) / data->blocks_per_group;

    // fs_data has to be set before reading the root inode, as the helpers rely on it.
    filesystem->fs_data = data;

    ext2_inode_t root_inode;

    if(ext2_read_inode(filesystem, EXT2_ROOT_INODE, &root_inode) != 0) {
        kfree(data);
        filesystem->fs_data = NULL;
        return -1;
    }

    filesystem->root = ext2_build_node(filesystem, EXT2_ROOT_INODE, "/", &root_inode);

    return 0;
}

static int32_t ext2_unmount(vfs_filesystem_t* filesystem) {
    kfree(filesystem->root);
    kfree(filesystem->fs_data);
    kfree(filesystem->operations);
    kfree(filesystem);

    return 0;
}

static size_t ext2_read_block(vfs_filesystem_t* filesystem, uint32_t block, void* buffer) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    return filesystem->volume->operations->read(filesystem->volume, block * data->block_size, data->block_size, (char*) buffer);
}

static int32_t ext2_read_bgd(vfs_filesystem_t* filesystem, uint32_t group, ext2_block_group_descriptor_t* out) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    if(group >= data->num_block_groups) {
        return -1;
    }

    uint32_t offset = group * sizeof(ext2_block_group_descriptor_t);
    uint32_t block = data->bgd_table_block + (offset / data->block_size);
    uint32_t offset_in_block = offset % data->block_size;

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ext2_read_block(filesystem, block, block_buffer);
    memcpy(out, block_buffer + offset_in_block, sizeof(ext2_block_group_descriptor_t));

    kfree(block_buffer);

    return 0;
}

static int32_t ext2_read_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, ext2_inode_t* out) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    if(inode_no == 0) {
        return -1;
    }

    uint32_t group = (inode_no - 1) / data->inodes_per_group;
    uint32_t index = (inode_no - 1) % data->inodes_per_group;

    ext2_block_group_descriptor_t bgd;

    if(ext2_read_bgd(filesystem, group, &bgd) != 0) {
        return -1;
    }

    uint32_t offset = index * data->inode_size;
    uint32_t block = bgd.bg_inode_table + (offset / data->block_size);
    uint32_t offset_in_block = offset % data->block_size;

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ext2_read_block(filesystem, block, block_buffer);

    // Only the base 128-byte inode is copied. A larger on-disk inode_size just changes the stride.
    memcpy(out, block_buffer + offset_in_block, sizeof(ext2_inode_t));

    kfree(block_buffer);

    return 0;
}

/**
 * Resolve the absolute block number of the index-th block of a file. Returns 0 for a sparse hole
 * or an out-of-range index. Handles the 12 direct as well as the single, double and triple
 * indirect block pointers.
 */
static uint32_t ext2_inode_block(vfs_filesystem_t* filesystem, ext2_inode_t* inode, uint32_t index) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    uint32_t pointers_per_block = data->block_size / sizeof(uint32_t);

    // 12 direct block pointers
    if(index < 12) {
        return inode->i_block[index];
    }

    index -= 12;

    uint32_t* block_buffer = (uint32_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    uint32_t result = 0;

    // Single indirect
    if(index < pointers_per_block) {
        if(inode->i_block[12] != 0) {
            ext2_read_block(filesystem, inode->i_block[12], block_buffer);
            result = block_buffer[index];
        }

        kfree(block_buffer);
        return result;
    }

    index -= pointers_per_block;

    // Double indirect
    if(index < pointers_per_block * pointers_per_block) {
        if(inode->i_block[13] != 0) {
            ext2_read_block(filesystem, inode->i_block[13], block_buffer);
            uint32_t single = block_buffer[index / pointers_per_block];

            if(single != 0) {
                ext2_read_block(filesystem, single, block_buffer);
                result = block_buffer[index % pointers_per_block];
            }
        }

        kfree(block_buffer);
        return result;
    }

    index -= pointers_per_block * pointers_per_block;

    // Triple indirect
    if(inode->i_block[14] != 0) {
        ext2_read_block(filesystem, inode->i_block[14], block_buffer);
        uint32_t double_block = block_buffer[index / (pointers_per_block * pointers_per_block)];

        if(double_block != 0) {
            ext2_read_block(filesystem, double_block, block_buffer);
            uint32_t remainder = index % (pointers_per_block * pointers_per_block);
            uint32_t single = block_buffer[remainder / pointers_per_block];

            if(single != 0) {
                ext2_read_block(filesystem, single, block_buffer);
                result = block_buffer[remainder % pointers_per_block];
            }
        }
    }

    kfree(block_buffer);
    return result;
}

/**
 * Allocate and populate a vfs_node from an inode. The type, size and ownership are taken directly
 * from the inode, and the matching operation table is selected based on the file type.
 */
static vfs_node_t* ext2_build_node(vfs_filesystem_t* filesystem, uint32_t inode_no, const char* name, ext2_inode_t* inode) {
    vfs_node_t* node = (vfs_node_t*) kmalloc(sizeof(vfs_node_t));

    if(!node) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strncpy(node->name, name, 256);

    node->permissions = inode->i_mode & 0x0FFF;
    node->uid = inode->i_uid;
    node->gid = inode->i_gid;
    node->length = inode->i_size;
    node->inode = inode_no;
    node->inode_data = NULL;
    node->link = NULL;
    node->filesystem = filesystem;

    switch(inode->i_mode & EXT2_S_IFMT) {
        case EXT2_S_IFDIR:
            node->type = VFS_DIRECTORY;
            node->operations = &ext2_directory_operations;
            break;
        case EXT2_S_IFLNK:
            node->type = VFS_SYMLINK;
            node->operations = &ext2_file_operations;
            break;
        default:
            node->type = VFS_FILE;
            node->operations = &ext2_file_operations;
            break;
    }

    return node;
}

static int32_t ext2_open(vfs_node_t* node) {
    // Directories are read on demand in readdir/finddir and need no state loaded here.
    if(node->type == VFS_DIRECTORY) {
        return 0;
    }

    ext2_inode_t* inode = (ext2_inode_t*) kmalloc(sizeof(ext2_inode_t));

    if(!inode) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    if(ext2_read_inode(node->filesystem, node->inode, inode) != 0) {
        kfree(inode);
        return -1;
    }

    node->inode_data = (void*) inode;

    return 0;
}

static int32_t ext2_close(vfs_node_t* node) {
    if(node->inode_data) {
        kfree(node->inode_data);
        node->inode_data = NULL;
    }

    return 0;
}

static int32_t ext2_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if(node->inode_data == NULL) {
        return -1;
    }

    ext2_inode_t* inode = (ext2_inode_t*) node->inode_data;
    ext2_fs_t* data = (ext2_fs_t*) node->filesystem->fs_data;

    if(offset > inode->i_size) {
        return 0;
    }

    if(offset + size > inode->i_size) {
        size = inode->i_size - offset;
    }

    if(size == 0) {
        return 0;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    size_t bytes_read = 0;

    while(bytes_read < size) {
        uint32_t file_block = (offset + bytes_read) / data->block_size;
        uint32_t offset_in_block = (offset + bytes_read) % data->block_size;

        uint32_t chunk = data->block_size - offset_in_block;

        if(chunk > size - bytes_read) {
            chunk = size - bytes_read;
        }

        uint32_t physical_block = ext2_inode_block(node->filesystem, inode, file_block);

        if(physical_block == 0) {
            // Sparse hole: the region reads back as zeros.
            memset((uint8_t*) buffer + bytes_read, 0, chunk);
        } else {
            ext2_read_block(node->filesystem, physical_block, block_buffer);
            memcpy((uint8_t*) buffer + bytes_read, block_buffer + offset_in_block, chunk);
        }

        bytes_read += chunk;
    }

    kfree(block_buffer);

    return (int32_t) bytes_read;
}

static vfs_dirent_t* ext2_readdir(vfs_node_t* node, uint32_t index) {
    ext2_fs_t* data = (ext2_fs_t*) node->filesystem->fs_data;

    ext2_inode_t inode;

    if(ext2_read_inode(node->filesystem, node->inode, &inode) != 0) {
        return NULL;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    uint32_t total_blocks = (inode.i_size + data->block_size - 1) / data->block_size;
    uint32_t current = 0;

    for(uint32_t b = 0; b < total_blocks; b++) {
        uint32_t physical_block = ext2_inode_block(node->filesystem, &inode, b);

        if(physical_block == 0) {
            continue;
        }

        ext2_read_block(node->filesystem, physical_block, block_buffer);

        uint32_t position = 0;

        while(position < data->block_size) {
            ext2_dir_entry_t* entry = (ext2_dir_entry_t*) (block_buffer + position);

            // A zero record length would loop forever on a corrupt directory block.
            if(entry->rec_len == 0) {
                break;
            }

            if(entry->inode != 0) {
                if(current == index) {
                    vfs_dirent_t* dirent = (vfs_dirent_t*) kmalloc(sizeof(vfs_dirent_t));

                    if(!dirent) {
                        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
                    }

                    uint8_t name_len = entry->name_len;

                    memcpy(dirent->name, (uint8_t*) entry + sizeof(ext2_dir_entry_t), name_len);
                    dirent->name[name_len] = '\0';
                    dirent->inode = entry->inode;

                    kfree(block_buffer);
                    return dirent;
                }

                current++;
            }

            position += entry->rec_len;
        }
    }

    kfree(block_buffer);
    return NULL;
}

static vfs_node_t* ext2_finddir(vfs_node_t* node, char* name) {
    ext2_fs_t* data = (ext2_fs_t*) node->filesystem->fs_data;

    ext2_inode_t inode;

    if(ext2_read_inode(node->filesystem, node->inode, &inode) != 0) {
        return NULL;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    uint32_t total_blocks = (inode.i_size + data->block_size - 1) / data->block_size;
    size_t name_len = strlen(name);

    for(uint32_t b = 0; b < total_blocks; b++) {
        uint32_t physical_block = ext2_inode_block(node->filesystem, &inode, b);

        if(physical_block == 0) {
            continue;
        }

        ext2_read_block(node->filesystem, physical_block, block_buffer);

        uint32_t position = 0;

        while(position < data->block_size) {
            ext2_dir_entry_t* entry = (ext2_dir_entry_t*) (block_buffer + position);

            if(entry->rec_len == 0) {
                break;
            }

            if(entry->inode != 0 && entry->name_len == name_len &&
               memcmp(name, (uint8_t*) entry + sizeof(ext2_dir_entry_t), name_len) == 0) {
                uint32_t child_inode_no = entry->inode;

                kfree(block_buffer);

                ext2_inode_t child_inode;

                if(ext2_read_inode(node->filesystem, child_inode_no, &child_inode) != 0) {
                    return NULL;
                }

                return ext2_build_node(node->filesystem, child_inode_no, name, &child_inode);
            }

            position += entry->rec_len;
        }
    }

    kfree(block_buffer);
    return NULL;
}

static int32_t ext2_rename(vfs_node_t* node, char* new_name) {
    // Unsupported because the driver is read-only for now.
    return -1;
}

static int32_t ext2_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    // Unsupported because the driver is read-only for now.
    return -1;
}

static int32_t ext2_create(vfs_node_t* node, char* name, uint32_t permissions) {
    // Unsupported because the driver is read-only for now.
    return -1;
}

static int32_t ext2_unlink(vfs_node_t* node, char* name) {
    // Unsupported because the driver is read-only for now.
    return -1;
}

static int32_t ext2_mkdir(vfs_node_t* node, char* name, uint32_t permissions) {
    // Unsupported because the driver is read-only for now.
    return -1;
}

static int32_t ext2_rmdir(vfs_node_t* node, char* name) {
    // Unsupported because the driver is read-only for now.
    return -1;
}
