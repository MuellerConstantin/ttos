#ifndef _KERNEL_FS_EXT2_H
#define _KERNEL_FS_EXT2_H

#include <stdint.h>
#include <stdbool.h>
#include <fs/vfs.h>
#include <device/volume.h>

/**
 * The ext2 superblock always starts 1024 bytes into the volume, independent of the block size.
 */
#define EXT2_SUPERBLOCK_OFFSET 1024

/**
 * Magic signature identifying an ext2 file system (stored in s_magic).
 */
#define EXT2_SUPER_MAGIC 0xEF53

/**
 * The root directory always lives at a fixed inode number.
 */
#define EXT2_ROOT_INODE 2

/**
 * i_mode format mask and the file type values relevant to this driver.
 */
#define EXT2_S_IFMT  0xF000     // Format mask
#define EXT2_S_IFREG 0x8000     // Regular file
#define EXT2_S_IFDIR 0x4000     // Directory
#define EXT2_S_IFLNK 0xA000     // Symbolic link

struct ext2_superblock {
    uint32_t s_inodes_count;        // Total number of inodes
    uint32_t s_blocks_count;        // Total number of blocks
    uint32_t s_r_blocks_count;      // Number of blocks reserved for the superuser
    uint32_t s_free_blocks_count;   // Number of free blocks
    uint32_t s_free_inodes_count;   // Number of free inodes
    uint32_t s_first_data_block;    // First data block (1 for 1 KiB blocks, 0 otherwise)
    uint32_t s_log_block_size;      // Block size is 1024 << s_log_block_size
    uint32_t s_log_frag_size;       // Fragment size
    uint32_t s_blocks_per_group;    // Number of blocks per group
    uint32_t s_frags_per_group;     // Number of fragments per group
    uint32_t s_inodes_per_group;    // Number of inodes per group
    uint32_t s_mtime;               // Last mount time
    uint32_t s_wtime;               // Last write time
    uint16_t s_mnt_count;           // Mount count since the last check
    uint16_t s_max_mnt_count;       // Max mount count before a check is forced
    uint16_t s_magic;               // Magic signature (EXT2_SUPER_MAGIC)
    uint16_t s_state;               // File system state
    uint16_t s_errors;              // Behaviour when detecting errors
    uint16_t s_minor_rev_level;     // Minor revision level
    uint32_t s_lastcheck;           // Time of the last check
    uint32_t s_checkinterval;       // Max time between checks
    uint32_t s_creator_os;          // OS that created the file system
    uint32_t s_rev_level;           // Revision level
    uint16_t s_def_resuid;          // Default uid for reserved blocks
    uint16_t s_def_resgid;          // Default gid for reserved blocks

    // EXT2_DYNAMIC_REV specific fields (only valid when s_rev_level >= 1)

    uint32_t s_first_ino;           // First non-reserved inode
    uint16_t s_inode_size;          // Size of the on-disk inode structure
    uint16_t s_block_group_nr;      // Block group hosting this superblock copy
    uint32_t s_feature_compat;      // Compatible feature set
    uint32_t s_feature_incompat;    // Incompatible feature set
    uint32_t s_feature_ro_compat;   // Read-only compatible feature set

    uint8_t  s_reserved[920];       // Padding to a full 1024-byte superblock
} __attribute__((packed));

typedef struct ext2_superblock ext2_superblock_t;

struct ext2_block_group_descriptor {
    uint32_t bg_block_bitmap;       // Block address of the block usage bitmap
    uint32_t bg_inode_bitmap;       // Block address of the inode usage bitmap
    uint32_t bg_inode_table;        // Starting block address of the inode table
    uint16_t bg_free_blocks_count;  // Number of unallocated blocks in the group
    uint16_t bg_free_inodes_count;  // Number of unallocated inodes in the group
    uint16_t bg_used_dirs_count;    // Number of directories in the group
    uint16_t bg_pad;                // Padding
    uint8_t  bg_reserved[12];       // Reserved
} __attribute__((packed));

typedef struct ext2_block_group_descriptor ext2_block_group_descriptor_t;

struct ext2_inode {
    uint16_t i_mode;                // Type and permissions
    uint16_t i_uid;                 // Owner user id
    uint32_t i_size;                // Lower 32 bits of the size in bytes
    uint32_t i_atime;               // Last access time
    uint32_t i_ctime;               // Creation time
    uint32_t i_mtime;               // Last modification time
    uint32_t i_dtime;               // Deletion time
    uint16_t i_gid;                 // Owner group id
    uint16_t i_links_count;         // Hard link count
    uint32_t i_blocks;              // Count of 512-byte sectors in use
    uint32_t i_flags;               // Flags
    uint32_t i_osd1;                // OS specific value #1
    uint32_t i_block[15];           // Block pointers (12 direct, 1 single, 1 double, 1 triple)
    uint32_t i_generation;          // Generation number
    uint32_t i_file_acl;            // Extended attribute block
    uint32_t i_dir_acl;             // Upper 32 bits of size (regular files) / directory ACL
    uint32_t i_faddr;               // Fragment address
    uint8_t  i_osd2[12];            // OS specific value #2
} __attribute__((packed));

typedef struct ext2_inode ext2_inode_t;

struct ext2_dir_entry {
    uint32_t inode;                 // Inode number (0 marks an unused entry)
    uint16_t rec_len;               // Total entry length, points to the next entry
    uint8_t  name_len;              // Length of the name (low 8 bits)
    uint8_t  file_type;             // File type (valid with the filetype feature)
    // The name follows immediately after this header, name_len bytes long.
} __attribute__((packed));

typedef struct ext2_dir_entry ext2_dir_entry_t;

/**
 * Probe for an ext2 file system.
 *
 * @param volume The volume to probe.
 * @return True if the file system is an ext2 file system, false otherwise.
 */
bool ext2_probe(volume_t* volume);

/**
 * Initialize an ext2 file system.
 *
 * @param volume The volume to initialize.
 * @return The mount point or NULL on error.
 */
vfs_filesystem_t* ext2_init(volume_t* volume);

#endif // _KERNEL_FS_EXT2_H
