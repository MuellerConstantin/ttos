#ifndef _LIBSYS_PROC_H
#define _LIBSYS_PROC_H

#include <stdint.h>

/**
 * Exits the current process with the given status code.
 * 
 * @param status The status code to exit with
 */
void _exit(int status);

#endif // _LIBSYS_PROC_H
