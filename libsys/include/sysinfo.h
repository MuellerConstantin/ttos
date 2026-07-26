#ifndef _LIBSYS_SYSINFO_H
#define _LIBSYS_SYSINFO_H

#include <stdint.h>
#include <stddef.h>

typedef struct osinfo osinfo_t;

struct osinfo {
    char name[16];
    char arch[16];
    char version[32];
    char platform[16];
};

typedef struct meminfo meminfo_t;

struct meminfo {
    size_t total;
    size_t free;
};

typedef struct terminfo terminfo_t;

struct terminfo {
    uint32_t rows;
    uint32_t cols;
};

/**
 * Gets system information.
 * 
 * @param info The system information.
 * @return 0 on success, -1 on error.
 */
int32_t sysinfo_get_osinfo(osinfo_t* info);

/**
 * Gets memory information.
 * 
 * @param info The memory information.
 * @return 0 on success, -1 on error.
 */
int32_t sysinfo_get_meminfo(meminfo_t* info);

/**
 * Gets terminal information (dimensions of the controlling terminal).
 *
 * @param info The terminal information.
 * @return 0 on success, -1 on error.
 */
int32_t sysinfo_get_terminfo(terminfo_t* info);

#endif // _LIBSYS_SYSINFO_H
