#ifndef _LIBSYS_SYSINFO_H
#define _LIBSYS_SYSINFO_H

#include <stdint.h>
#include <stddef.h>
#include <ttos/syscall.h>

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
 * Gets kernel heap information.
 *
 * @param info The memory information (total/free) of the kernel heap.
 * @return 0 on success, -1 on error.
 */
int32_t sysinfo_get_kheapinfo(meminfo_t* info);

/**
 * Gets terminal information (dimensions of the controlling terminal).
 *
 * @param info The terminal information.
 * @return 0 on success, -1 on error.
 */
int32_t sysinfo_get_terminfo(terminfo_t* info);

/**
 * Gets the system uptime in seconds.
 *
 * @return The system uptime in seconds.
 */
uint32_t sysinfo_get_uptime(void);

#endif // _LIBSYS_SYSINFO_H
