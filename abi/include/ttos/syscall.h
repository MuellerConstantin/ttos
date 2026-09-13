#ifndef _TTOS_SYSCALL_H
#define _TTOS_SYSCALL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Interrupt vector a system call is issued through.
#define SYSCALL_INTERRUPT 0x80

// System call numbers, passed in eax.
#define SYSCALL_READ 0x00
#define SYSCALL_WRITE 0x01
#define SYSCALL_OPEN 0x02
#define SYSCALL_CLOSE 0x03
#define SYSCALL_GET_OSINFO 0x04
#define SYSCALL_GET_MEMINFO 0x05
#define SYSCALL_GET_TERMINFO 0x06
#define SYSCALL_ALLOC_HEAP 0x0A
#define SYSCALL_EXIT 0x0B
#define SYSCALL_OPENDIR 0x0C
#define SYSCALL_READDIR 0x0D
#define SYSCALL_CLOSEDIR 0x0E
#define SYSCALL_LSVOL 0x0F
#define SYSCALL_POWEROFF 0x10
#define SYSCALL_LSDEV 0x11
#define SYSCALL_LSMNT 0x12
#define SYSCALL_MOUNT 0x13
#define SYSCALL_UNMOUNT 0x14
#define SYSCALL_DMESG 0x15
#define SYSCALL_UPTIME 0x16
#define SYSCALL_MEMMAP 0x17
#define SYSCALL_GET_KHEAPINFO 0x18
#define SYSCALL_SPAWN 0x19
#define SYSCALL_UNLINK 0x1A
#define SYSCALL_RMDIR 0x1B
#define SYSCALL_MKDIR 0x1C
#define SYSCALL_CHDIR 0x1D
#define SYSCALL_GETCWD 0x1E
#define SYSCALL_STAT 0x1F
#define SYSCALL_DEVREAD 0x20
#define SYSCALL_DEVWRITE 0x21
#define SYSCALL_RESCAN 0x22
#define SYSCALL_GET_FSINFO 0x23

// Longest path the kernel accepts or reports, including the terminating NUL.
#define PATH_MAX 256

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

typedef struct fsinfo fsinfo_t;

/**
 * Describes how much of a volume the file system on it occupies. Counted in
 * bytes rather than in blocks or inodes, as not every file system knows those.
 */
struct fsinfo {
    uint32_t total;
    uint32_t free;
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
    uint32_t size;

    // File system the volume carries, empty if none is recognized.
    char fs_type[16];

    // Label of that file system, empty if it carries none.
    char label[16];
};

// Device types, as reported in devinfo.
#define DEVICE_TYPE_UNKNOWN     0x0000
#define DEVICE_TYPE_KEYBOARD    0x0100
#define DEVICE_TYPE_STORAGE     0x0200
#define DEVICE_TYPE_VIDEO       0x0300
#define DEVICE_TYPE_CONTROLLER  0x0400
#define DEVICE_TYPE_SERIAL      0x0500
#define DEVICE_TYPE_TIMER       0x0600
#define DEVICE_TYPE_RESERVED    0xFF00

// Bus types a device can be attached to, as reported in devinfo.
#define DEVICE_BUS_TYPE_PLATFORM    0x00
#define DEVICE_BUS_TYPE_ISA         0x01
#define DEVICE_BUS_TYPE_PCI         0x02
#define DEVICE_BUS_TYPE_USB         0x03
#define DEVICE_BUS_TYPE_ATA         0x04
#define DEVICE_BUS_TYPE_PS2         0x05
#define DEVICE_BUS_TYPE_RESERVED    0xFF

typedef struct devinfo devinfo_t;

/**
 * Describes a device in the system's device tree.
 */
struct devinfo {
    char name[64];
    char id[16];
    uint16_t type;
    uint8_t bus_type;

    // Distance from the root of the device tree, zero for the root itself.
    uint8_t depth;

    /*
     * Bit n tells whether the device on the path from the root at depth n is
     * the last of its siblings. A listing needs that for every level it draws
     * through, to know whether that branch still continues further down. Bit 0
     * belongs to the root and is never set.
     */
    uint32_t last_child_mask;
};

typedef struct mntinfo mntinfo_t;

/**
 * Describes a mount point in the system's mount table.
 */
struct mntinfo {
    char drive;

    // Short id of the mounted volume, as listed in volinfo.
    char volume_id[16];

    // Name of the file system type the volume is mounted as.
    char fs_type[16];
};

// File types, as reported in fileinfo.
#define FILE_TYPE_FILE      1
#define FILE_TYPE_DIRECTORY 2
#define FILE_TYPE_SYMLINK   3

typedef struct fileinfo fileinfo_t;

/**
 * Describes a file or directory. Together, volume_id and inode identify it:
 * two paths that agree on both name the same object of the same type.
 */
struct fileinfo {
    uint32_t type;
    uint32_t size;
    uint32_t inode;

    // Short id of the volume the file lives on, as listed in volinfo.
    char volume_id[16];

    uint32_t permissions;
    uint32_t uid;
    uint32_t gid;
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
