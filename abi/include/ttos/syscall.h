#ifndef _TTOS_SYSCALL_H
#define _TTOS_SYSCALL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct osinfo osinfo_t;

/**
 * Describes the running operating system.
 */
struct osinfo {
    char name[16];
    char arch[16];
    char version[32];
    char platform[16];
};

typedef struct meminfo meminfo_t;

/**
 * Describes the usage of a memory pool, either the physical memory or the
 * kernel heap.
 */
struct meminfo {
    size_t total;
    size_t free;
};

typedef struct terminfo terminfo_t;

/**
 * Describes the dimensions of the terminal a process is attached to.
 */
struct terminfo {
    uint32_t rows;
    uint32_t cols;
};

typedef struct dirent dirent_t;

/**
 * A single entry of a directory.
 */
struct dirent {
    char name[256];
    uint32_t inode;
};

typedef struct volinfo volinfo_t;

/**
 * Describes a volume in the system's volume list.
 */
struct volinfo {
    char name[64];
    char id[16];
};

typedef struct devinfo devinfo_t;

/**
 * Describes a device in the system's device tree.
 */
struct devinfo {
    char name[64];
    char id[16];
};

typedef struct mntinfo mntinfo_t;

/**
 * Describes a mount point in the system's mount table.
 */
struct mntinfo {
    char drive;
};

typedef struct kmsg_entry kmsg_entry_t;

/**
 * A single message from the kernel log.
 */
struct kmsg_entry {
    char level[16];
    char message[256];
};

typedef struct memregion memregion_t;

/**
 * A single region of the physical memory map.
 */
struct memregion {
    uint32_t base;
    uint32_t length;
    uint32_t type;
};

#ifdef __cplusplus
}
#endif

#endif // _TTOS_SYSCALL_H
