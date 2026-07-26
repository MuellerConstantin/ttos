#ifndef _LIBSYS_POWER_H
#define _LIBSYS_POWER_H

#include <stdint.h>

/**
 * Powers off the system.
 *
 * On success the machine powers off and this call never returns. It only
 * returns -1 when the power off could not be performed.
 *
 * @return -1 on error, otherwise it does not return.
 */
int32_t power_off(void);

#endif // _LIBSYS_POWER_H
