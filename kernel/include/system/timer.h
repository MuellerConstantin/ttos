#ifndef _KERNEL_SYSTEM_TIMER_H
#define _KERNEL_SYSTEM_TIMER_H

#include <stdint.h>

typedef void (*timer_wakeup_listener_t)();

typedef struct timer_wakeup_info timer_wakeup_info_t;

struct timer_wakeup_info {
    double seconds;
    uint32_t jiffies;
    timer_wakeup_listener_t listener;
};

/**
 * Initialize the system timer.
 */
void timer_init();

/**
 * Get the system uptime in seconds.
 * 
 * @return The system uptime in seconds.
 */
uint32_t timer_get_uptime();

/**
 * Put the current process to sleep for at least the given time, rounded up
 * to whole timer ticks. Other processes run in the meantime. Returns early
 * if the process is asked to terminate.
 *
 * @param milliseconds The time to sleep.
 * @return 0 once the time has passed, or -1 if the sleep was cut short.
 */
int32_t timer_sleep(uint32_t milliseconds);

/**
 * Register a wakeup call to be called every given amount of seconds.
 * 
 * @param seconds The amount of seconds to wait between each wakeup call.
 * @param listener The listener to call on wakeup.
 */
void timer_register_wakeup_call(double seconds, timer_wakeup_listener_t listener);

#endif // _KERNEL_SYSTEM_TIMER_H
