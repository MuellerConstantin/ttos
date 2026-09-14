#include <volio.h>
#include <mntio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Creates the ext2 file system this system reads. The layout is not
 * configurable: the driver understands one block size, one inode size and one
 * feature set, and anything else it would refuse or, worse, misread. The
 * on-disk structures are declared here rather than shared with the kernel; the
 * format is fixed from outside and cannot drift, but the subset of features
 * used has to stay the one the driver knows.
 */

#define MKFS_BLOCK_SIZE 1024
#define MKFS_INODE_SIZE 128
#define MKFS_BLOCKS_PER_GROUP (8 * MKFS_BLOCK_SIZE)

/** One inode per this many bytes of volume, which is what mke2fs settles on by default. */
#define MKFS_BYTES_PER_INODE 16384

/** Inodes below this one are reserved; the root directory is one of them. */
#define MKFS_ROOT_INODE 2
#define MKFS_FIRST_INODE 11

/** Share of the blocks kept for the superuser. */
#define MKFS_RESERVED_PERCENT 5

/** With 1 KiB blocks the superblock is a block of its own, and the first one. */
#define MKFS_SUPERBLOCK_OFFSET 1024
#define MKFS_FIRST_DATA_BLOCK 1

#define MKFS_MAGIC 0xEF53

/** Revision that keeps the inode size and the first inode in the superblock. */
#define MKFS_REV_DYNAMIC 1

/*
 * The feature set the driver is built for. filetype puts the type of an entry
 * into the directory itself; sparse_super places backup superblocks in a few
 * groups instead of all of them; large_file only says a file may exceed 2 GiB.
 */
#define MKFS_FEATURE_INCOMPAT_FILETYPE 0x0002
#define MKFS_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001
#define MKFS_FEATURE_RO_COMPAT_LARGE_FILE 0x0002

#define MKFS_STATE_CLEAN 1
#define MKFS_ERRORS_CONTINUE 1
#define MKFS_CREATOR_LINUX 0

/** Mount counts are not tracked, so no check is ever forced. */
#define MKFS_MAX_MOUNT_COUNT 0xFFFF

#define MKFS_MODE_DIRECTORY 0x4000
#define MKFS_TYPE_DIRECTORY 2

/** Sectors of 512 bytes a block occupies, which is what i_blocks counts. */
#define MKFS_SECTORS_PER_BLOCK (MKFS_BLOCK_SIZE / 512)

struct superblock {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t  s_uuid[16];
    char     s_volume_name[16];
    char     s_last_mounted[64];
    uint8_t  s_reserved[824];
} __attribute__((packed));

struct group_descriptor {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint8_t  bg_reserved[12];
} __attribute__((packed));

struct inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint8_t  i_osd2[12];
} __attribute__((packed));

struct dir_entry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
} __attribute__((packed));

/** The layout derived from the size of the volume, shared by every step below. */
typedef struct {
    const char* id;
    uint32_t total_blocks;
    uint32_t num_groups;
    uint32_t inodes_per_group;
    uint32_t inode_table_blocks;
    uint32_t descriptor_blocks;
    uint32_t root_block;
    uint32_t lost_found_block;
    uint32_t free_blocks;
    uint32_t free_inodes;
} mkfs_layout_t;

static unsigned char block[MKFS_BLOCK_SIZE];
static struct superblock superblock;

/*
 * Zeroes the inode tables are cleared with. A table is written in runs of this
 * size rather than block by block: every write is a round trip to the drive
 * that ends with its cache being flushed, and the tables are nearly all of
 * what formatting writes.
 */
#define MKFS_ZERO_BLOCKS 64

static unsigned char zeros[MKFS_ZERO_BLOCKS * MKFS_BLOCK_SIZE];

static uint32_t mkfs_divide_up(uint32_t value, uint32_t divisor) {
    return (value + divisor - 1) / divisor;
}

/*
 * Whether a group carries a copy of the superblock. With sparse_super only
 * group 0, group 1 and the groups whose number is a power of three, five or
 * seven do, which is what keeps the copies from filling a large file system.
 */
static int mkfs_has_superblock(uint32_t group) {
    if (group <= 1) {
        return 1;
    }

    for (uint32_t base = 3; base <= 7; base += 2) {
        for (uint32_t power = base; power <= group; power *= base) {
            if (power == group) {
                return 1;
            }
        }
    }

    return 0;
}

/** Blocks a group spends on metadata before its data begins. */
static uint32_t mkfs_group_overhead(const mkfs_layout_t* layout, uint32_t group) {
    uint32_t overhead = mkfs_has_superblock(group) ? 1 + layout->descriptor_blocks : 0;

    // The two bitmaps and the inode table follow in every group.
    return overhead + 2 + layout->inode_table_blocks;
}

/** First block of a group, counted from the start of the volume. */
static uint32_t mkfs_group_first_block(uint32_t group) {
    return MKFS_FIRST_DATA_BLOCK + group * MKFS_BLOCKS_PER_GROUP;
}

/** Blocks a group holds, which is fewer than a full group for the last one. */
static uint32_t mkfs_group_blocks(const mkfs_layout_t* layout, uint32_t group) {
    uint32_t first = mkfs_group_first_block(group);
    uint32_t rest = layout->total_blocks - first;

    return rest < MKFS_BLOCKS_PER_GROUP ? rest : MKFS_BLOCKS_PER_GROUP;
}

static int mkfs_write_block(const mkfs_layout_t* layout, uint32_t number, const void* data) {
    size_t offset = (size_t) number * MKFS_BLOCK_SIZE;

    if (volio_write(layout->id, offset, data, MKFS_BLOCK_SIZE) != MKFS_BLOCK_SIZE) {
        printf("mkfs: cannot write block %d\n", (int) number);
        return -1;
    }

    return 0;
}

/** Clears a run of blocks, as many per write as the zero buffer holds. */
static int mkfs_write_zeros(const mkfs_layout_t* layout, uint32_t first, uint32_t count) {
    while (count > 0) {
        uint32_t run = count < MKFS_ZERO_BLOCKS ? count : MKFS_ZERO_BLOCKS;
        size_t offset = (size_t) first * MKFS_BLOCK_SIZE;
        size_t size = (size_t) run * MKFS_BLOCK_SIZE;

        if (volio_write(layout->id, offset, zeros, size) != (int32_t) size) {
            printf("mkfs: cannot write block %d\n", (int) first);
            return -1;
        }

        first += run;
        count -= run;
    }

    return 0;
}

static void mkfs_bitmap_set(unsigned char* bitmap, uint32_t index) {
    bitmap[index / 8] |= (unsigned char) (1 << (index % 8));
}

/** Fills in the superblock as it is stored in group 0 and in every backup. */
static void mkfs_build_superblock(const mkfs_layout_t* layout, const char* label) {
    memset(&superblock, 0, sizeof(superblock));

    superblock.s_inodes_count = layout->inodes_per_group * layout->num_groups;
    superblock.s_blocks_count = layout->total_blocks;
    superblock.s_r_blocks_count = layout->total_blocks * MKFS_RESERVED_PERCENT / 100;
    superblock.s_free_blocks_count = layout->free_blocks;
    superblock.s_free_inodes_count = layout->free_inodes;
    superblock.s_first_data_block = MKFS_FIRST_DATA_BLOCK;
    superblock.s_log_block_size = 0;
    superblock.s_log_frag_size = 0;
    superblock.s_blocks_per_group = MKFS_BLOCKS_PER_GROUP;
    superblock.s_frags_per_group = MKFS_BLOCKS_PER_GROUP;
    superblock.s_inodes_per_group = layout->inodes_per_group;
    superblock.s_max_mnt_count = MKFS_MAX_MOUNT_COUNT;
    superblock.s_magic = MKFS_MAGIC;
    superblock.s_state = MKFS_STATE_CLEAN;
    superblock.s_errors = MKFS_ERRORS_CONTINUE;
    superblock.s_creator_os = MKFS_CREATOR_LINUX;
    superblock.s_rev_level = MKFS_REV_DYNAMIC;
    superblock.s_first_ino = MKFS_FIRST_INODE;
    superblock.s_inode_size = MKFS_INODE_SIZE;
    superblock.s_feature_incompat = MKFS_FEATURE_INCOMPAT_FILETYPE;
    superblock.s_feature_ro_compat = MKFS_FEATURE_RO_COMPAT_SPARSE_SUPER | MKFS_FEATURE_RO_COMPAT_LARGE_FILE;

    if (label) {
        strncpy(superblock.s_volume_name, label, sizeof(superblock.s_volume_name));
    }
}

/** Writes the superblock into group 0 and into every group that backs it up. */
static int mkfs_write_superblocks(const mkfs_layout_t* layout) {
    for (uint32_t group = 0; group < layout->num_groups; group++) {
        if (!mkfs_has_superblock(group)) {
            continue;
        }

        superblock.s_block_group_nr = (uint16_t) group;

        /*
         * The first superblock sits at a fixed offset rather than at the start
         * of its block, as the block in front of it holds whatever boot code a
         * file system at the start of a disk needs to leave room for.
         */
        size_t offset = group == 0 ? MKFS_SUPERBLOCK_OFFSET : (size_t) mkfs_group_first_block(group) * MKFS_BLOCK_SIZE;

        if (volio_write(layout->id, offset, &superblock, sizeof(superblock)) != (int32_t) sizeof(superblock)) {
            printf("mkfs: cannot write the superblock of group %d\n", (int) group);
            return -1;
        }
    }

    return 0;
}

/** Writes the descriptor table, which every group with a superblock repeats. */
static int mkfs_write_descriptors(const mkfs_layout_t* layout) {
    for (uint32_t index = 0; index < layout->descriptor_blocks; index++) {
        memset(block, 0, sizeof(block));

        uint32_t per_block = MKFS_BLOCK_SIZE / sizeof(struct group_descriptor);
        uint32_t first = index * per_block;

        for (uint32_t offset = 0; offset < per_block && first + offset < layout->num_groups; offset++) {
            uint32_t group = first + offset;
            uint32_t group_start = mkfs_group_first_block(group) + (mkfs_has_superblock(group) ? 1 + layout->descriptor_blocks : 0);
            uint32_t used_blocks = mkfs_group_overhead(layout, group) + (group == 0 ? 2 : 0);

            struct group_descriptor* descriptor = (struct group_descriptor*) block + offset;

            descriptor->bg_block_bitmap = group_start;
            descriptor->bg_inode_bitmap = group_start + 1;
            descriptor->bg_inode_table = group_start + 2;
            descriptor->bg_free_blocks_count = (uint16_t) (mkfs_group_blocks(layout, group) - used_blocks);
            descriptor->bg_free_inodes_count = (uint16_t) (layout->inodes_per_group - (group == 0 ? MKFS_FIRST_INODE : 0));

            // The root directory and lost+found both live in the first group.
            descriptor->bg_used_dirs_count = group == 0 ? 2 : 0;
        }

        for (uint32_t group = 0; group < layout->num_groups; group++) {
            if (!mkfs_has_superblock(group)) {
                continue;
            }

            uint32_t table_block = mkfs_group_first_block(group) + 1;

            if (mkfs_write_block(layout, table_block + index, block) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

/** Writes the two bitmaps and the empty inode table of one group. */
static int mkfs_write_group(const mkfs_layout_t* layout, uint32_t group) {
    uint32_t group_start = mkfs_group_first_block(group) + (mkfs_has_superblock(group) ? 1 + layout->descriptor_blocks : 0);
    uint32_t used_blocks = mkfs_group_overhead(layout, group) + (group == 0 ? 2 : 0);
    uint32_t group_blocks = mkfs_group_blocks(layout, group);

    // Block bitmap: the metadata at the front is taken, and so is everything past the end of a short group.
    memset(block, 0, sizeof(block));

    for (uint32_t index = 0; index < used_blocks; index++) {
        mkfs_bitmap_set(block, index);
    }

    for (uint32_t index = group_blocks; index < MKFS_BLOCKS_PER_GROUP; index++) {
        mkfs_bitmap_set(block, index);
    }

    if (mkfs_write_block(layout, group_start, block) != 0) {
        return -1;
    }

    // Inode bitmap: the reserved inodes are taken in the first group, the rest is free.
    memset(block, 0, sizeof(block));

    if (group == 0) {
        for (uint32_t index = 0; index < MKFS_FIRST_INODE; index++) {
            mkfs_bitmap_set(block, index);
        }
    }

    for (uint32_t index = layout->inodes_per_group; index < MKFS_BLOCK_SIZE * 8; index++) {
        mkfs_bitmap_set(block, index);
    }

    if (mkfs_write_block(layout, group_start + 1, block) != 0) {
        return -1;
    }

    return mkfs_write_zeros(layout, group_start + 2, layout->inode_table_blocks);
}

/*
 * Places one inode in the inode table. An inode is found by its number alone:
 * which block of the table it lands in and where inside that block both follow
 * from it, and the two do not have to agree.
 */
static int mkfs_place_inode(const mkfs_layout_t* layout, uint32_t table, uint32_t number, const struct inode* source) {
    uint32_t per_block = MKFS_BLOCK_SIZE / MKFS_INODE_SIZE;
    uint32_t index = number - 1;
    uint32_t target = table + index / per_block;
    size_t offset = (size_t) target * MKFS_BLOCK_SIZE;

    // The table was written out zeroed, so what is there stays apart from this one inode.
    if (volio_read(layout->id, offset, block, MKFS_BLOCK_SIZE) != MKFS_BLOCK_SIZE) {
        printf("mkfs: cannot read block %d of the inode table\n", (int) target);
        return -1;
    }

    memcpy(block + (index % per_block) * MKFS_INODE_SIZE, source, sizeof(struct inode));

    return mkfs_write_block(layout, target, block);
}

/** Writes the root directory and lost+found, their inodes and their contents. */
static int mkfs_write_root(const mkfs_layout_t* layout) {
    uint32_t inode_table = mkfs_group_first_block(0) + 1 + layout->descriptor_blocks + 2;
    struct inode entry_inode;

    // Three links: the entry in its parent, its own dot, and the dotdot of lost+found.
    memset(&entry_inode, 0, sizeof(entry_inode));

    entry_inode.i_mode = MKFS_MODE_DIRECTORY | 0755;
    entry_inode.i_size = MKFS_BLOCK_SIZE;
    entry_inode.i_links_count = 3;
    entry_inode.i_blocks = MKFS_SECTORS_PER_BLOCK;
    entry_inode.i_block[0] = layout->root_block;

    if (mkfs_place_inode(layout, inode_table, MKFS_ROOT_INODE, &entry_inode) != 0) {
        return -1;
    }

    memset(&entry_inode, 0, sizeof(entry_inode));

    entry_inode.i_mode = MKFS_MODE_DIRECTORY | 0700;
    entry_inode.i_size = MKFS_BLOCK_SIZE;
    entry_inode.i_links_count = 2;
    entry_inode.i_blocks = MKFS_SECTORS_PER_BLOCK;
    entry_inode.i_block[0] = layout->lost_found_block;

    if (mkfs_place_inode(layout, inode_table, MKFS_FIRST_INODE, &entry_inode) != 0) {
        return -1;
    }

    // The root directory: itself, its parent, which is itself, and lost+found.
    memset(block, 0, sizeof(block));

    struct dir_entry* entry = (struct dir_entry*) block;

    entry->inode = MKFS_ROOT_INODE;
    entry->rec_len = 12;
    entry->name_len = 1;
    entry->file_type = MKFS_TYPE_DIRECTORY;
    memcpy(block + sizeof(struct dir_entry), ".", 1);

    entry = (struct dir_entry*) (block + 12);
    entry->inode = MKFS_ROOT_INODE;
    entry->rec_len = 12;
    entry->name_len = 2;
    entry->file_type = MKFS_TYPE_DIRECTORY;
    memcpy(block + 12 + sizeof(struct dir_entry), "..", 2);

    entry = (struct dir_entry*) (block + 24);
    entry->inode = MKFS_FIRST_INODE;
    entry->rec_len = (uint16_t) (MKFS_BLOCK_SIZE - 24);
    entry->name_len = 10;
    entry->file_type = MKFS_TYPE_DIRECTORY;
    memcpy(block + 24 + sizeof(struct dir_entry), "lost+found", 10);

    if (mkfs_write_block(layout, layout->root_block, block) != 0) {
        return -1;
    }

    memset(block, 0, sizeof(block));

    entry = (struct dir_entry*) block;
    entry->inode = MKFS_FIRST_INODE;
    entry->rec_len = 12;
    entry->name_len = 1;
    entry->file_type = MKFS_TYPE_DIRECTORY;
    memcpy(block + sizeof(struct dir_entry), ".", 1);

    entry = (struct dir_entry*) (block + 12);
    entry->inode = MKFS_ROOT_INODE;
    entry->rec_len = (uint16_t) (MKFS_BLOCK_SIZE - 12);
    entry->name_len = 2;
    entry->file_type = MKFS_TYPE_DIRECTORY;
    memcpy(block + 12 + sizeof(struct dir_entry), "..", 2);

    return mkfs_write_block(layout, layout->lost_found_block, block);
}

/** Looks the volume up and reports its size, or 0 when there is no such volume. */
static uint32_t mkfs_volume_size(const char* id) {
    volinfo_t volume;

    for (uint32_t index = 0; volio_list(index, &volume) == 0; index++) {
        if (strcmp(volume.id, id) == 0) {
            return volume.size;
        }
    }

    return 0;
}

/** Whether the volume is mounted, in which case it must not be written over. */
static int mkfs_is_mounted(const char* id) {
    mntinfo_t mount;

    for (uint32_t index = 0; mntio_list(index, &mount) == 0; index++) {
        if (strcmp(mount.volume_id, id) == 0) {
            return 1;
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        puts("usage: mkfs <volume> [<label>]\n");
        return 1;
    }

    mkfs_layout_t layout;

    layout.id = argv[1];

    uint32_t size = mkfs_volume_size(layout.id);

    if (size == 0) {
        printf("mkfs: no such volume: %s\n", layout.id);
        return 1;
    }

    if (mkfs_is_mounted(layout.id)) {
        printf("mkfs: %s is mounted\n", layout.id);
        return 1;
    }

    layout.total_blocks = size / MKFS_BLOCK_SIZE;

    if (layout.total_blocks <= MKFS_FIRST_DATA_BLOCK + 16) {
        printf("mkfs: %s is too small for a file system\n", layout.id);
        return 1;
    }

    layout.num_groups = mkfs_divide_up(layout.total_blocks - MKFS_FIRST_DATA_BLOCK, MKFS_BLOCKS_PER_GROUP);
    layout.descriptor_blocks = mkfs_divide_up(layout.num_groups * sizeof(struct group_descriptor), MKFS_BLOCK_SIZE);

    // The bitmap addresses one inode per bit, so a group holds a multiple of eight.
    uint32_t inodes = mkfs_divide_up(size / MKFS_BYTES_PER_INODE, layout.num_groups);

    layout.inodes_per_group = mkfs_divide_up(inodes, 8) * 8;
    layout.inode_table_blocks = mkfs_divide_up(layout.inodes_per_group * MKFS_INODE_SIZE, MKFS_BLOCK_SIZE);

    // The root directory and lost+found take the first two data blocks of group 0.
    layout.root_block = mkfs_group_first_block(0) + mkfs_group_overhead(&layout, 0);
    layout.lost_found_block = layout.root_block + 1;

    layout.free_blocks = 0;
    layout.free_inodes = 0;

    for (uint32_t group = 0; group < layout.num_groups; group++) {
        layout.free_blocks += mkfs_group_blocks(&layout, group) - mkfs_group_overhead(&layout, group);
        layout.free_inodes += layout.inodes_per_group;
    }

    layout.free_blocks -= 2;
    layout.free_inodes -= MKFS_FIRST_INODE;

    printf("%d blocks in %d groups, %d inodes per group\n", (int) layout.total_blocks,
           (int) layout.num_groups, (int) layout.inodes_per_group);

    for (uint32_t group = 0; group < layout.num_groups; group++) {
        if (mkfs_write_group(&layout, group) != 0) {
            return 1;
        }
    }

    mkfs_build_superblock(&layout, argc == 3 ? argv[2] : NULL);

    if (mkfs_write_superblocks(&layout) != 0 || mkfs_write_descriptors(&layout) != 0 || mkfs_write_root(&layout) != 0) {
        return 1;
    }

    printf("%s formatted\n", layout.id);

    return 0;
}
