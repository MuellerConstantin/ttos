/**
 * @file wait_queue.h
 * @brief Queues of processes sleeping until an event occurs.
 *
 * A wait queue belongs to whatever produces the event: the terminal for
 * input, a process for the exit of its children. A process sleeps on the
 * queue while the condition it needs is false and is woken together with all
 * other sleepers when the producer signals; every sleeper re-checks its own
 * condition afterwards, as it may have been woken for another reason.
 *
 * Kernel code always runs with interrupts disabled, so checking a condition
 * and going to sleep on its queue cannot be interleaved with the interrupt
 * that would make it true: no wake-up can be lost.
 */

#ifndef _KERNEL_SYSTEM_WAIT_QUEUE_H
#define _KERNEL_SYSTEM_WAIT_QUEUE_H

#include <util/linked_list.h>

typedef struct wait_queue wait_queue_t;

struct wait_queue {
    linked_list_t waiters;
};

/**
 * Initialize an empty wait queue.
 *
 * @param queue The queue.
 */
void wait_queue_init(wait_queue_t* queue);

/**
 * Put the current process to sleep on a queue and run something else until
 * the queue is woken. Returns once the process is scheduled again; the caller
 * re-checks its condition and sleeps again if it still does not hold.
 *
 * @param queue The queue to sleep on.
 */
void wait_queue_sleep(wait_queue_t* queue);

/**
 * Wake every process sleeping on a queue.
 *
 * @param queue The queue.
 */
void wait_queue_wake_all(wait_queue_t* queue);

#endif // _KERNEL_SYSTEM_WAIT_QUEUE_H
