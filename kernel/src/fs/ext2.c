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
    uint32_t first_inode;           // First non-reserved inode (with rev-0 fallback)
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
static int32_t ext2_truncate(vfs_node_t* node, uint32_t length);
static int32_t ext2_create(vfs_node_t* node, char* name, uint32_t permissions);
static int32_t ext2_unlink(vfs_node_t* node, char* name);
static int32_t ext2_mkdir(vfs_node_t* node, char* name, uint32_t permissions);
static int32_t ext2_rmdir(vfs_node_t* node, char* name);
static vfs_dirent_t* ext2_readdir(vfs_node_t* node, uint32_t index);
static vfs_node_t* ext2_finddir(vfs_node_t* node, char* name);
static int32_t ext2_rename(vfs_node_t* node, char* new_name);

// Internal helpers

static size_t ext2_read_block(vfs_filesystem_t* filesystem, uint32_t block, void* buffer);
static size_t ext2_write_block(vfs_filesystem_t* filesystem, uint32_t block, void* buffer);
static int32_t ext2_read_bgd(vfs_filesystem_t* filesystem, uint32_t group, ext2_block_group_descriptor_t* out);
static int32_t ext2_write_bgd(vfs_filesystem_t* filesystem, uint32_t group, ext2_block_group_descriptor_t* bgd);
static int32_t ext2_write_superblock(vfs_filesystem_t* filesystem);
static int32_t ext2_read_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, ext2_inode_t* out);
static int32_t ext2_write_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, ext2_inode_t* inode);
static bool ext2_bitmap_test(uint8_t* bitmap, uint32_t index);
static void ext2_bitmap_set(uint8_t* bitmap, uint32_t index);
static void ext2_bitmap_clear(uint8_t* bitmap, uint32_t index);
static uint32_t ext2_bitmap_find_free(uint8_t* bitmap, uint32_t count, uint32_t from);
static uint32_t ext2_alloc_block(vfs_filesystem_t* filesystem);
static int32_t ext2_free_block(vfs_filesystem_t* filesystem, uint32_t block);
static uint32_t ext2_alloc_inode(vfs_filesystem_t* filesystem, bool directory);
static int32_t ext2_free_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, bool directory);
static uint32_t ext2_inode_block(vfs_filesystem_t* filesystem, ext2_inode_t* inode, uint32_t index);
static uint32_t ext2_indirect_slot(vfs_filesystem_t* filesystem, ext2_inode_t* inode, uint32_t indirect_block, uint32_t slot);
static uint32_t ext2_inode_alloc_block(vfs_filesystem_t* filesystem, ext2_inode_t* inode, uint32_t index);
static void ext2_free_indirect(vfs_filesystem_t* filesystem, uint32_t block, uint32_t level);
static uint32_t ext2_dir_entry_size(uint8_t name_len);
static int32_t ext2_dir_insert(vfs_filesystem_t* filesystem, uint32_t dir_inode_no, const char* name, uint32_t inode_no, uint8_t file_type);
static bool ext2_dir_is_empty(vfs_filesystem_t* filesystem, uint32_t dir_inode_no);
static uint32_t ext2_dir_remove(vfs_filesystem_t* filesystem, uint32_t dir_inode_no, const char* name);
static void ext2_release_blocks(vfs_filesystem_t* filesystem, ext2_inode_t* inode);
static int32_t ext2_release_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, ext2_inode_t* inode, bool directory);
static vfs_node_t* ext2_build_node(vfs_filesystem_t* filesystem, uint32_t inode_no, const char* name, ext2_inode_t* inode);

static vfs_node_operations_t ext2_directory_operations = {
    .open = &ext2_open,
    .close = &ext2_close,
    .rename = &ext2_rename,
    .read = NULL,
    .write = NULL,
    .truncate = NULL,
    .create = &ext2_create,
    .unlink = &ext2_unlink,
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
    .truncate = &ext2_truncate,
    .create = NULL,
    .unlink = NULL,
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

    ext2_mountpoint->type = "ext2";
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
    data->first_inode = (data->superblock.s_rev_level >= 1) ? data->superblock.s_first_ino : 11;
    data->bgd_table_block = data->superblock.s_first_data_block + 1;
    data->num_block_groups = (data->superblock.s_blocks_count - data->superblock.s_first_data_block
        + data->blocks_per_group - 1) / data->blocks_per_group;

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

static size_t ext2_write_block(vfs_filesystem_t* filesystem, uint32_t block, void* buffer) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    return filesystem->volume->operations->write(filesystem->volume, block * data->block_size, data->block_size, (char*) buffer);
}

/**
 * Flush the cached superblock back to the volume. The cached copy spans the full 1024 bytes that
 * were read at mount time, so fields this driver does not know about are preserved.
 */
static int32_t ext2_write_superblock(vfs_filesystem_t* filesystem) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    size_t written = filesystem->volume->operations->write(filesystem->volume, EXT2_SUPERBLOCK_OFFSET, sizeof(ext2_superblock_t), (char*) &data->superblock);

    return (written == sizeof(ext2_superblock_t)) ? 0 : -1;
}

static int32_t ext2_write_bgd(vfs_filesystem_t* filesystem, uint32_t group, ext2_block_group_descriptor_t* bgd) {
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

    // Read-modify-write, because the block holds the descriptors of the neighbouring groups too.
    ext2_read_block(filesystem, block, block_buffer);
    memcpy(block_buffer + offset_in_block, bgd, sizeof(ext2_block_group_descriptor_t));
    ext2_write_block(filesystem, block, block_buffer);

    kfree(block_buffer);

    return 0;
}

static int32_t ext2_write_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, ext2_inode_t* inode) {
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

    // Read-modify-write, mirroring ext2_read_inode: only the base 128-byte inode is touched, so a
    // larger on-disk inode keeps whatever follows it.
    ext2_read_block(filesystem, block, block_buffer);
    memcpy(block_buffer + offset_in_block, inode, sizeof(ext2_inode_t));
    ext2_write_block(filesystem, block, block_buffer);

    kfree(block_buffer);

    return 0;
}

static bool ext2_bitmap_test(uint8_t* bitmap, uint32_t index) {
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

static void ext2_bitmap_set(uint8_t* bitmap, uint32_t index) {
    bitmap[index / 8] |= 1 << (index % 8);
}

static void ext2_bitmap_clear(uint8_t* bitmap, uint32_t index) {
    bitmap[index / 8] &= ~(1 << (index % 8));
}

/**
 * Find the first clear bit in a bitmap, starting the search at the from-th bit. Returns count when
 * all of the first count bits are set.
 */
static uint32_t ext2_bitmap_find_free(uint8_t* bitmap, uint32_t count, uint32_t from) {
    for(uint32_t index = from; index < count; index++) {
        // Skip over fully occupied bytes, but only where the whole byte is in range.
        if(index % 8 == 0 && index + 8 <= count && bitmap[index / 8] == 0xFF) {
            index += 7;
            continue;
        }

        if(!ext2_bitmap_test(bitmap, index)) {
            return index;
        }
    }

    return count;
}

/**
 * Allocate a single block. Scans the block bitmaps group by group, marks the first free block as
 * used and keeps the free counters of the group descriptor and the superblock in sync. The block
 * is zeroed before it is handed out, so callers can use it as an indirect or directory block
 * without clearing it first and no stale data leaks into a new file.
 *
 * @return The allocated block number or 0 if the file system is full. Block 0 is never allocatable
 *         (it holds the superblock or precedes s_first_data_block), so it works as an error value.
 */
static uint32_t ext2_alloc_block(vfs_filesystem_t* filesystem) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    if(data->superblock.s_free_blocks_count == 0) {
        return 0;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    for(uint32_t group = 0; group < data->num_block_groups; group++) {
        ext2_block_group_descriptor_t bgd;

        if(ext2_read_bgd(filesystem, group, &bgd) != 0) {
            break;
        }

        if(bgd.bg_free_blocks_count == 0) {
            continue;
        }

        // The last group is usually shorter than a full one, so its bitmap has padding bits that
        // must not be handed out.
        uint32_t first_block = data->superblock.s_first_data_block + group * data->blocks_per_group;

        if(first_block >= data->superblock.s_blocks_count) {
            break;
        }

        uint32_t blocks_in_group = data->superblock.s_blocks_count - first_block;

        if(blocks_in_group > data->blocks_per_group) {
            blocks_in_group = data->blocks_per_group;
        }

        ext2_read_block(filesystem, bgd.bg_block_bitmap, block_buffer);

        uint32_t index = ext2_bitmap_find_free(block_buffer, blocks_in_group, 0);

        if(index >= blocks_in_group) {
            // The descriptor claims free blocks the bitmap does not have. Skip the group instead of
            // trusting the counter.
            continue;
        }

        ext2_bitmap_set(block_buffer, index);
        ext2_write_block(filesystem, bgd.bg_block_bitmap, block_buffer);

        bgd.bg_free_blocks_count--;
        ext2_write_bgd(filesystem, group, &bgd);

        data->superblock.s_free_blocks_count--;
        ext2_write_superblock(filesystem);

        memset(block_buffer, 0, data->block_size);
        ext2_write_block(filesystem, first_block + index, block_buffer);

        kfree(block_buffer);

        return first_block + index;
    }

    kfree(block_buffer);

    return 0;
}

/**
 * Release a block and give it back to its group. Freeing a block that is already marked free is
 * refused, as counting it twice would corrupt the free counters.
 */
static int32_t ext2_free_block(vfs_filesystem_t* filesystem, uint32_t block) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    if(block < data->superblock.s_first_data_block || block >= data->superblock.s_blocks_count) {
        return -1;
    }

    uint32_t group = (block - data->superblock.s_first_data_block) / data->blocks_per_group;
    uint32_t index = (block - data->superblock.s_first_data_block) % data->blocks_per_group;

    ext2_block_group_descriptor_t bgd;

    if(ext2_read_bgd(filesystem, group, &bgd) != 0) {
        return -1;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ext2_read_block(filesystem, bgd.bg_block_bitmap, block_buffer);

    if(!ext2_bitmap_test(block_buffer, index)) {
        kfree(block_buffer);
        return -1;
    }

    ext2_bitmap_clear(block_buffer, index);
    ext2_write_block(filesystem, bgd.bg_block_bitmap, block_buffer);

    kfree(block_buffer);

    bgd.bg_free_blocks_count++;
    ext2_write_bgd(filesystem, group, &bgd);

    data->superblock.s_free_blocks_count++;
    ext2_write_superblock(filesystem);

    return 0;
}

/**
 * Allocate a single inode. Works like ext2_alloc_block, but additionally maintains the directory
 * count of the group, which e2fsck checks against the inodes it finds. The on-disk inode is zeroed
 * so a caller that only fills in part of it cannot inherit the block pointers of a deleted file.
 *
 * @param directory True if the inode will hold a directory.
 * @return The allocated inode number or 0 if no inode is free. Inode 0 does not exist, so it works
 *         as an error value.
 */
static uint32_t ext2_alloc_inode(vfs_filesystem_t* filesystem, bool directory) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    if(data->superblock.s_free_inodes_count == 0) {
        return 0;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    for(uint32_t group = 0; group < data->num_block_groups; group++) {
        ext2_block_group_descriptor_t bgd;

        if(ext2_read_bgd(filesystem, group, &bgd) != 0) {
            break;
        }

        if(bgd.bg_free_inodes_count == 0) {
            continue;
        }

        ext2_read_block(filesystem, bgd.bg_inode_bitmap, block_buffer);

        // The reserved inodes all live in the first group and are marked used by mkfs, but the
        // search starts behind them so a stale bitmap cannot hand out the root inode.
        uint32_t from = (group == 0) ? data->first_inode - 1 : 0;
        uint32_t index = ext2_bitmap_find_free(block_buffer, data->inodes_per_group, from);

        if(index >= data->inodes_per_group) {
            continue;
        }

        ext2_bitmap_set(block_buffer, index);
        ext2_write_block(filesystem, bgd.bg_inode_bitmap, block_buffer);

        bgd.bg_free_inodes_count--;

        if(directory) {
            bgd.bg_used_dirs_count++;
        }

        ext2_write_bgd(filesystem, group, &bgd);

        data->superblock.s_free_inodes_count--;
        ext2_write_superblock(filesystem);

        kfree(block_buffer);

        uint32_t inode_no = group * data->inodes_per_group + index + 1;

        ext2_inode_t inode;
        memset(&inode, 0, sizeof(ext2_inode_t));
        ext2_write_inode(filesystem, inode_no, &inode);

        return inode_no;
    }

    kfree(block_buffer);

    return 0;
}

/**
 * Release an inode and give it back to its group. The reserved inodes, the root inode among them,
 * are never freed. Detaching the data blocks is the caller's job, as only it knows whether the
 * inode still has links left.
 *
 * @param directory True if the inode held a directory, so bg_used_dirs_count is corrected.
 */
static int32_t ext2_free_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, bool directory) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    if(inode_no < data->first_inode || inode_no > data->superblock.s_inodes_count) {
        return -1;
    }

    uint32_t group = (inode_no - 1) / data->inodes_per_group;
    uint32_t index = (inode_no - 1) % data->inodes_per_group;

    ext2_block_group_descriptor_t bgd;

    if(ext2_read_bgd(filesystem, group, &bgd) != 0) {
        return -1;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ext2_read_block(filesystem, bgd.bg_inode_bitmap, block_buffer);

    if(!ext2_bitmap_test(block_buffer, index)) {
        kfree(block_buffer);
        return -1;
    }

    ext2_bitmap_clear(block_buffer, index);
    ext2_write_block(filesystem, bgd.bg_inode_bitmap, block_buffer);

    kfree(block_buffer);

    bgd.bg_free_inodes_count++;

    if(directory && bgd.bg_used_dirs_count > 0) {
        bgd.bg_used_dirs_count--;
    }

    ext2_write_bgd(filesystem, group, &bgd);

    data->superblock.s_free_inodes_count++;
    ext2_write_superblock(filesystem);

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
 * Follow one slot of an indirect block, allocating the block it points at when the slot is still
 * empty. The indirect block is written back whenever a new pointer is stored, and i_blocks is
 * charged for the allocation.
 *
 * @return The block number the slot points at or 0 when the file system is full.
 */
static uint32_t ext2_indirect_slot(vfs_filesystem_t* filesystem, ext2_inode_t* inode, uint32_t indirect_block, uint32_t slot) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    uint32_t* block_buffer = (uint32_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ext2_read_block(filesystem, indirect_block, block_buffer);

    uint32_t block = block_buffer[slot];

    if(block == 0) {
        block = ext2_alloc_block(filesystem);

        if(block != 0) {
            block_buffer[slot] = block;
            ext2_write_block(filesystem, indirect_block, block_buffer);

            inode->i_blocks += data->block_size / 512;
        }
    }

    kfree(block_buffer);

    return block;
}

/**
 * Resolve the absolute block number of the index-th block of a file, allocating the block and every
 * indirect block on the way to it when they are still missing. The counterpart of ext2_inode_block,
 * which reports holes rather than filling them.
 *
 * The caller owns the inode struct and is responsible for writing it back, as i_block and i_blocks
 * are updated in place here. Indirect blocks count towards i_blocks as well, which is what ext2
 * expects and what e2fsck verifies.
 *
 * @return The block number or 0 when the file system is full or the index is out of range.
 */
static uint32_t ext2_inode_alloc_block(vfs_filesystem_t* filesystem, ext2_inode_t* inode, uint32_t index) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    uint32_t pointers_per_block = data->block_size / sizeof(uint32_t);
    uint32_t sectors_per_block = data->block_size / 512;

    // 12 direct block pointers
    if(index < 12) {
        if(inode->i_block[index] == 0) {
            uint32_t block = ext2_alloc_block(filesystem);

            if(block == 0) {
                return 0;
            }

            inode->i_block[index] = block;
            inode->i_blocks += sectors_per_block;
        }

        return inode->i_block[index];
    }

    index -= 12;

    /*
     * Pick the indirection level and break the remaining index down into one slot per level. The
     * walk below is then the same for all three levels.
     */
    uint32_t root;
    uint32_t slots[3];
    uint32_t depth;

    if(index < pointers_per_block) {
        root = 12;
        depth = 1;
        slots[0] = index;
    } else if(index - pointers_per_block < pointers_per_block * pointers_per_block) {
        index -= pointers_per_block;

        root = 13;
        depth = 2;
        slots[0] = index / pointers_per_block;
        slots[1] = index % pointers_per_block;
    } else {
        index -= pointers_per_block + pointers_per_block * pointers_per_block;

        if(index >= pointers_per_block * pointers_per_block * pointers_per_block) {
            return 0;
        }

        root = 14;
        depth = 3;
        slots[0] = index / (pointers_per_block * pointers_per_block);
        slots[1] = (index % (pointers_per_block * pointers_per_block)) / pointers_per_block;
        slots[2] = index % pointers_per_block;
    }

    if(inode->i_block[root] == 0) {
        uint32_t block = ext2_alloc_block(filesystem);

        if(block == 0) {
            return 0;
        }

        // ext2_alloc_block hands out a zeroed block, so the fresh indirect block reads back as a
        // table of empty slots without any extra clearing.
        inode->i_block[root] = block;
        inode->i_blocks += sectors_per_block;
    }

    uint32_t block = inode->i_block[root];

    for(uint32_t level = 0; level < depth; level++) {
        block = ext2_indirect_slot(filesystem, inode, block, slots[level]);

        if(block == 0) {
            return 0;
        }
    }

    return block;
}

/**
 * Free an indirect block and everything below it. Level 1 is a single indirect block whose slots
 * are data blocks, level 2 a double and level 3 a triple indirect block.
 */
static void ext2_free_indirect(vfs_filesystem_t* filesystem, uint32_t block, uint32_t level) {
    if(block == 0) {
        return;
    }

    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    uint32_t pointers_per_block = data->block_size / sizeof(uint32_t);

    uint32_t* block_buffer = (uint32_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ext2_read_block(filesystem, block, block_buffer);

    for(uint32_t slot = 0; slot < pointers_per_block; slot++) {
        if(block_buffer[slot] == 0) {
            continue;
        }

        if(level > 1) {
            ext2_free_indirect(filesystem, block_buffer[slot], level - 1);
        } else {
            ext2_free_block(filesystem, block_buffer[slot]);
        }
    }

    kfree(block_buffer);

    ext2_free_block(filesystem, block);
}

/**
 * Size an on-disk directory entry needs for a given name: the header plus the name, rounded up to
 * the 4-byte alignment ext2 requires.
 */
static uint32_t ext2_dir_entry_size(uint8_t name_len) {
    return (sizeof(ext2_dir_entry_t) + name_len + 3) & ~3u;
}

/**
 * Add an entry to a directory. Walks the directory blocks looking for a record with enough slack,
 * either an entry marked unused or the padding behind a live entry, and splits that record in two.
 * When no block has room, a further block is appended to the directory and the new entry spans it
 * whole.
 *
 * @return 0 on success or -1 on error.
 */
static int32_t ext2_dir_insert(vfs_filesystem_t* filesystem, uint32_t dir_inode_no, const char* name, uint32_t inode_no, uint8_t file_type) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    size_t name_len = strlen(name);

    if(name_len == 0 || name_len > 255) {
        return -1;
    }

    uint32_t needed = ext2_dir_entry_size((uint8_t) name_len);

    if(needed > data->block_size) {
        return -1;
    }

    ext2_inode_t dir_inode;

    if(ext2_read_inode(filesystem, dir_inode_no, &dir_inode) != 0) {
        return -1;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    uint32_t total_blocks = (dir_inode.i_size + data->block_size - 1) / data->block_size;

    for(uint32_t b = 0; b < total_blocks; b++) {
        uint32_t physical_block = ext2_inode_block(filesystem, &dir_inode, b);

        if(physical_block == 0) {
            continue;
        }

        ext2_read_block(filesystem, physical_block, block_buffer);

        uint32_t position = 0;

        while(position < data->block_size) {
            ext2_dir_entry_t* entry = (ext2_dir_entry_t*) (block_buffer + position);

            if(entry->rec_len == 0) {
                break;
            }

            // An unused entry can be taken over whole; a live one only lends its trailing padding.
            uint32_t used = (entry->inode == 0) ? 0 : ext2_dir_entry_size(entry->name_len);

            if(entry->rec_len - used >= needed) {
                ext2_dir_entry_t* target;

                if(used == 0) {
                    target = entry;
                } else {
                    target = (ext2_dir_entry_t*) (block_buffer + position + used);
                    target->rec_len = entry->rec_len - used;
                    entry->rec_len = used;
                }

                target->inode = inode_no;
                target->name_len = (uint8_t) name_len;
                target->file_type = file_type;

                memcpy((uint8_t*) target + sizeof(ext2_dir_entry_t), name, name_len);

                ext2_write_block(filesystem, physical_block, block_buffer);

                kfree(block_buffer);
                return 0;
            }

            position += entry->rec_len;
        }
    }

    /*
     * No room anywhere: append a block. A directory is always a whole number of blocks long, so
     * the new block sits at index i_size / block_size and grows i_size by exactly one block.
     */
    uint32_t physical_block = ext2_inode_alloc_block(filesystem, &dir_inode, dir_inode.i_size / data->block_size);

    if(physical_block == 0) {
        kfree(block_buffer);
        return -1;
    }

    memset(block_buffer, 0, data->block_size);

    ext2_dir_entry_t* entry = (ext2_dir_entry_t*) block_buffer;

    entry->inode = inode_no;
    entry->rec_len = (uint16_t) data->block_size;
    entry->name_len = (uint8_t) name_len;
    entry->file_type = file_type;

    memcpy((uint8_t*) entry + sizeof(ext2_dir_entry_t), name, name_len);

    ext2_write_block(filesystem, physical_block, block_buffer);

    kfree(block_buffer);

    dir_inode.i_size += data->block_size;

    return ext2_write_inode(filesystem, dir_inode_no, &dir_inode);
}

/**
 * Check whether a directory holds anything besides the . and .. entries.
 */
static bool ext2_dir_is_empty(vfs_filesystem_t* filesystem, uint32_t dir_inode_no) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    ext2_inode_t inode;

    if(ext2_read_inode(filesystem, dir_inode_no, &inode) != 0) {
        return false;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    uint32_t total_blocks = (inode.i_size + data->block_size - 1) / data->block_size;
    bool empty = true;

    for(uint32_t b = 0; b < total_blocks && empty; b++) {
        uint32_t physical_block = ext2_inode_block(filesystem, &inode, b);

        if(physical_block == 0) {
            continue;
        }

        ext2_read_block(filesystem, physical_block, block_buffer);

        uint32_t position = 0;

        while(position < data->block_size) {
            ext2_dir_entry_t* entry = (ext2_dir_entry_t*) (block_buffer + position);

            if(entry->rec_len == 0) {
                break;
            }

            if(entry->inode != 0) {
                const char* name = (const char*) entry + sizeof(ext2_dir_entry_t);

                bool dot = (entry->name_len == 1 && name[0] == '.');
                bool dotdot = (entry->name_len == 2 && name[0] == '.' && name[1] == '.');

                if(!dot && !dotdot) {
                    empty = false;
                    break;
                }
            }

            position += entry->rec_len;
        }
    }

    kfree(block_buffer);

    return empty;
}

/**
 * Remove an entry from a directory. The record is folded into the one in front of it, which is how
 * ext2 reclaims the space; an entry at the start of a block has no predecessor to grow and is only
 * marked unused, leaving a gap that ext2_dir_insert can take over again.
 *
 * @return The inode the entry pointed at, or 0 if the name is not in the directory.
 */
static uint32_t ext2_dir_remove(vfs_filesystem_t* filesystem, uint32_t dir_inode_no, const char* name) {
    ext2_fs_t* data = (ext2_fs_t*) filesystem->fs_data;

    ext2_inode_t dir_inode;

    if(ext2_read_inode(filesystem, dir_inode_no, &dir_inode) != 0) {
        return 0;
    }

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    uint32_t total_blocks = (dir_inode.i_size + data->block_size - 1) / data->block_size;
    size_t name_len = strlen(name);

    for(uint32_t b = 0; b < total_blocks; b++) {
        uint32_t physical_block = ext2_inode_block(filesystem, &dir_inode, b);

        if(physical_block == 0) {
            continue;
        }

        ext2_read_block(filesystem, physical_block, block_buffer);

        uint32_t position = 0;
        uint32_t previous = 0;
        bool has_previous = false;

        while(position < data->block_size) {
            ext2_dir_entry_t* entry = (ext2_dir_entry_t*) (block_buffer + position);

            if(entry->rec_len == 0) {
                break;
            }

            if(entry->inode != 0 && entry->name_len == name_len &&
               memcmp(name, (uint8_t*) entry + sizeof(ext2_dir_entry_t), name_len) == 0) {
                uint32_t inode_no = entry->inode;

                if(has_previous) {
                    ext2_dir_entry_t* predecessor = (ext2_dir_entry_t*) (block_buffer + previous);
                    predecessor->rec_len += entry->rec_len;
                } else {
                    entry->inode = 0;
                }

                ext2_write_block(filesystem, physical_block, block_buffer);

                kfree(block_buffer);
                return inode_no;
            }

            previous = position;
            has_previous = true;

            position += entry->rec_len;
        }
    }

    kfree(block_buffer);
    return 0;
}

/**
 * Hand every block of an inode back and reset its size. Writing the inode is left to the caller,
 * which usually has further changes to make to it.
 */
static void ext2_release_blocks(vfs_filesystem_t* filesystem, ext2_inode_t* inode) {
    for(uint32_t index = 0; index < 12; index++) {
        if(inode->i_block[index] != 0) {
            ext2_free_block(filesystem, inode->i_block[index]);
        }
    }

    ext2_free_indirect(filesystem, inode->i_block[12], 1);
    ext2_free_indirect(filesystem, inode->i_block[13], 2);
    ext2_free_indirect(filesystem, inode->i_block[14], 3);

    memset(inode->i_block, 0, sizeof(inode->i_block));

    inode->i_size = 0;
    inode->i_blocks = 0;
}

/**
 * Drop the last link to an inode: release its blocks, stamp it as deleted and give it back to its
 * block group.
 *
 * @param directory True if the inode held a directory, so the group's directory count is corrected.
 */
static int32_t ext2_release_inode(vfs_filesystem_t* filesystem, uint32_t inode_no, ext2_inode_t* inode, bool directory) {
    ext2_release_blocks(filesystem, inode);

    inode->i_links_count = 0;

    /*
     * A freed inode has to carry a non-zero deletion time, otherwise it reads as a live inode that
     * lost its links. There is no wall clock in the system, so the smallest value that satisfies
     * the rule stands in, matching the epoch that every other timestamp here already carries.
     */
    inode->i_dtime = EXT2_UNKNOWN_TIME;

    if(ext2_write_inode(filesystem, inode_no, inode) != 0) {
        return -1;
    }

    return ext2_free_inode(filesystem, inode_no, directory);
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
    // Not implemented yet.
    return -1;
}

static int32_t ext2_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer) {
    if(node->inode_data == NULL) {
        return -1;
    }

    if(size == 0) {
        return 0;
    }

    ext2_inode_t* inode = (ext2_inode_t*) node->inode_data;
    ext2_fs_t* data = (ext2_fs_t*) node->filesystem->fs_data;

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    size_t bytes_written = 0;

    while(bytes_written < size) {
        uint32_t file_block = (offset + bytes_written) / data->block_size;
        uint32_t offset_in_block = (offset + bytes_written) % data->block_size;

        uint32_t chunk = data->block_size - offset_in_block;

        if(chunk > size - bytes_written) {
            chunk = size - bytes_written;
        }

        uint32_t physical_block = ext2_inode_alloc_block(node->filesystem, inode, file_block);

        if(physical_block == 0) {
            // Out of space. Report the part that made it rather than discarding it silently.
            break;
        }

        if(chunk == data->block_size) {
            memcpy(block_buffer, (uint8_t*) buffer + bytes_written, chunk);
        } else {
            // Partial block, so the surrounding bytes have to be preserved. A freshly allocated
            // block reads back as zeros, which is exactly what a sparse region should look like.
            ext2_read_block(node->filesystem, physical_block, block_buffer);
            memcpy(block_buffer + offset_in_block, (uint8_t*) buffer + bytes_written, chunk);
        }

        ext2_write_block(node->filesystem, physical_block, block_buffer);

        bytes_written += chunk;
    }

    kfree(block_buffer);

    if(bytes_written == 0) {
        return -1;
    }

    if(offset + bytes_written > inode->i_size) {
        inode->i_size = offset + bytes_written;
        node->length = inode->i_size;
    }

    // i_block and i_blocks were updated in place while allocating, so the inode has to go back to
    // disk even when the file did not grow.
    if(ext2_write_inode(node->filesystem, node->inode, inode) != 0) {
        return -1;
    }

    return (int32_t) bytes_written;
}

static int32_t ext2_truncate(vfs_node_t* node, uint32_t length) {
    if(node->inode_data == NULL) {
        return -1;
    }

    /*
     * Only truncating to zero is supported. Cutting a file short at an arbitrary length also has to
     * prune the indirect blocks that fall empty in the process, and no caller needs that yet.
     */
    if(length != 0) {
        return -1;
    }

    ext2_inode_t* inode = (ext2_inode_t*) node->inode_data;

    ext2_release_blocks(node->filesystem, inode);

    node->length = 0;

    return ext2_write_inode(node->filesystem, node->inode, inode);
}

static int32_t ext2_create(vfs_node_t* node, char* name, uint32_t permissions) {
    if(node->type != VFS_DIRECTORY) {
        return -1;
    }

    // Refuse to shadow an existing name instead of leaving two entries behind that resolve
    // differently depending on which one finddir reaches first.
    vfs_node_t* existing = ext2_finddir(node, name);

    if(existing) {
        kfree(existing);
        return -1;
    }

    uint32_t inode_no = ext2_alloc_inode(node->filesystem, false);

    if(inode_no == 0) {
        return -1;
    }

    ext2_inode_t inode;

    memset(&inode, 0, sizeof(ext2_inode_t));

    inode.i_mode = EXT2_S_IFREG | (permissions & 0x0FFF);
    inode.i_links_count = 1;

    if(ext2_write_inode(node->filesystem, inode_no, &inode) != 0) {
        ext2_free_inode(node->filesystem, inode_no, false);
        return -1;
    }

    if(ext2_dir_insert(node->filesystem, node->inode, name, inode_no, EXT2_FT_REG_FILE) != 0) {
        // The inode never became reachable, so handing it straight back keeps the counters right.
        ext2_free_inode(node->filesystem, inode_no, false);
        return -1;
    }

    return 0;
}

static int32_t ext2_unlink(vfs_node_t* node, char* name) {
    if(node->type != VFS_DIRECTORY) {
        return -1;
    }

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return -1;
    }

    // A directory has to be taken down with rmdir, which also corrects the parent's link count.
    vfs_node_t* target = ext2_finddir(node, name);

    if(!target) {
        return -1;
    }

    bool is_directory = (target->type == VFS_DIRECTORY);

    kfree(target);

    if(is_directory) {
        return -1;
    }

    uint32_t inode_no = ext2_dir_remove(node->filesystem, node->inode, name);

    if(inode_no == 0) {
        return -1;
    }

    ext2_inode_t inode;

    if(ext2_read_inode(node->filesystem, inode_no, &inode) != 0) {
        return -1;
    }

    if(inode.i_links_count > 0) {
        inode.i_links_count--;
    }

    // Another name still reaches the inode, so only the link count changes.
    if(inode.i_links_count > 0) {
        return ext2_write_inode(node->filesystem, inode_no, &inode);
    }

    return ext2_release_inode(node->filesystem, inode_no, &inode, false);
}

static int32_t ext2_mkdir(vfs_node_t* node, char* name, uint32_t permissions) {
    if(node->type != VFS_DIRECTORY) {
        return -1;
    }

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return -1;
    }

    ext2_fs_t* data = (ext2_fs_t*) node->filesystem->fs_data;

    vfs_node_t* existing = ext2_finddir(node, name);

    if(existing) {
        kfree(existing);
        return -1;
    }

    uint32_t inode_no = ext2_alloc_inode(node->filesystem, true);

    if(inode_no == 0) {
        return -1;
    }

    ext2_inode_t inode;

    memset(&inode, 0, sizeof(ext2_inode_t));

    inode.i_mode = EXT2_S_IFDIR | (permissions & 0x0FFF);

    // The "." entry below is a link to the directory itself, the entry in the parent is the second.
    inode.i_links_count = 2;

    uint32_t block = ext2_inode_alloc_block(node->filesystem, &inode, 0);

    if(block == 0) {
        ext2_free_inode(node->filesystem, inode_no, true);
        return -1;
    }

    // A directory is always a whole number of blocks long, however little of it is in use.
    inode.i_size = data->block_size;

    uint8_t* block_buffer = (uint8_t*) kmalloc(data->block_size);

    if(!block_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    memset(block_buffer, 0, data->block_size);

    ext2_dir_entry_t* self = (ext2_dir_entry_t*) block_buffer;

    self->inode = inode_no;
    self->rec_len = (uint16_t) ext2_dir_entry_size(1);
    self->name_len = 1;
    self->file_type = EXT2_FT_DIR;

    memcpy((uint8_t*) self + sizeof(ext2_dir_entry_t), ".", 1);

    // The parent entry takes the rest of the block, as the last record of a block always does.
    ext2_dir_entry_t* parent_entry = (ext2_dir_entry_t*) (block_buffer + self->rec_len);

    parent_entry->inode = node->inode;
    parent_entry->rec_len = (uint16_t) (data->block_size - self->rec_len);
    parent_entry->name_len = 2;
    parent_entry->file_type = EXT2_FT_DIR;

    memcpy((uint8_t*) parent_entry + sizeof(ext2_dir_entry_t), "..", 2);

    ext2_write_block(node->filesystem, block, block_buffer);

    kfree(block_buffer);

    if(ext2_write_inode(node->filesystem, inode_no, &inode) != 0 ||
       ext2_dir_insert(node->filesystem, node->inode, name, inode_no, EXT2_FT_DIR) != 0) {
        // The directory never became reachable, so everything it took goes straight back.
        ext2_release_blocks(node->filesystem, &inode);
        ext2_free_inode(node->filesystem, inode_no, true);
        return -1;
    }

    // The new directory's ".." is an additional link to the parent.
    ext2_inode_t parent;

    if(ext2_read_inode(node->filesystem, node->inode, &parent) != 0) {
        return -1;
    }

    parent.i_links_count++;

    return ext2_write_inode(node->filesystem, node->inode, &parent);
}

static int32_t ext2_rmdir(vfs_node_t* node, char* name) {
    if(node->type != VFS_DIRECTORY) {
        return -1;
    }

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return -1;
    }

    vfs_node_t* target = ext2_finddir(node, name);

    if(!target) {
        return -1;
    }

    bool is_directory = (target->type == VFS_DIRECTORY);
    uint32_t inode_no = target->inode;

    kfree(target);

    if(!is_directory) {
        return -1;
    }

    if(!ext2_dir_is_empty(node->filesystem, inode_no)) {
        return -1;
    }

    if(ext2_dir_remove(node->filesystem, node->inode, name) == 0) {
        return -1;
    }

    ext2_inode_t inode;

    if(ext2_read_inode(node->filesystem, inode_no, &inode) != 0) {
        return -1;
    }

    if(ext2_release_inode(node->filesystem, inode_no, &inode, true) != 0) {
        return -1;
    }

    /*
     * The removed directory's ".." was a link to the parent, so the parent loses one now that the
     * child is gone.
     */
    ext2_inode_t parent;

    if(ext2_read_inode(node->filesystem, node->inode, &parent) != 0) {
        return -1;
    }

    if(parent.i_links_count > 0) {
        parent.i_links_count--;
    }

    return ext2_write_inode(node->filesystem, node->inode, &parent);
}
