#include <system/timer.h>
#include <system/kpanic.h>
#include <arch/i386/isr.h>
#include <drivers/pit/8253.h>
#include <system/process.h>
#include <system/wait_queue.h>
#include <util/linked_list.h>

static volatile uint32_t timer_jiffies = 0;
static volatile uint16_t timer_hz = 0;
static linked_list_t *timer_wakeup_calls = NULL;

/*
 * Processes in timer_sleep, and the earliest tick any of them is due at. The
 * tick wakes them all once that is reached; a sleeper whose own deadline lies
 * further out lowers the mark again and goes back to sleep.
 */
static wait_queue_t timer_sleepers;
static uint32_t timer_next_wakeup = UINT32_MAX;

static void timer_set_frequency(uint16_t hz);
static void timer_interrupt_handler(isr_cpu_state_t *state);

void timer_init() {
    wait_queue_init(&timer_sleepers);

    timer_set_frequency(100);
    isr_register_listener(PROGRAMMABLE_INTERRUPT_TIMER_INTERRUPT, timer_interrupt_handler);

    pit_8253_claim_device();
}

static void timer_set_frequency(uint16_t hz) {
    pit_8253_init(PIT_8253_COUNTER_0, hz);

    timer_jiffies = 0;
    timer_hz = hz;

    timer_wakeup_calls = linked_list_create();

    if(timer_wakeup_calls == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }
}

static void timer_interrupt_handler(isr_cpu_state_t *state) {
    timer_jiffies++;

    process_tick();

    if(timer_jiffies >= timer_next_wakeup) {
        timer_next_wakeup = UINT32_MAX;
        wait_queue_wake_all(&timer_sleepers);
    }

    linked_list_foreach(timer_wakeup_calls, node) {
        timer_wakeup_info_t *info = (timer_wakeup_info_t*) node->data;

        if(timer_jiffies >= info->jiffies) {
            info->listener();
            info->jiffies = timer_jiffies + (info->seconds * timer_hz);
        }
    }
}

uint32_t timer_get_uptime() {
    return timer_jiffies / timer_hz;
}

int32_t timer_sleep(uint32_t milliseconds) {
    // Rounded up: a sleep shorter than a tick still waits for the next one.
    uint32_t end = timer_jiffies + (milliseconds * timer_hz + 999) / 1000;

    /*
     * Without a process to put to sleep (kernel initialization, the idle
     * context) there is nothing to switch to; spin on the tick instead, which
     * needs interrupts to be enabled.
     */
    if(process_get_current() == NULL) {
        while(timer_jiffies < end);

        return 0;
    }

    while(timer_jiffies < end) {
        if(process_kill_pending()) {
            return -1;
        }

        if(end < timer_next_wakeup) {
            timer_next_wakeup = end;
        }

        wait_queue_sleep(&timer_sleepers);
    }

    return 0;
}

void timer_register_wakeup_call(double seconds, timer_wakeup_listener_t listener) {
    timer_wakeup_info_t *info = kmalloc(sizeof(timer_wakeup_info_t));

    if(info == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    info->seconds = seconds;
    info->jiffies = timer_jiffies + (seconds * timer_hz);
    info->listener = listener;

    linked_list_node_t *node = linked_list_create_node(info);

    if(node == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    linked_list_append(timer_wakeup_calls, node);
}
