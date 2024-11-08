#include <system/timer.h>
#include <system/kpanic.h>
#include <arch/i386/isr.h>
#include <drivers/pit/8253.h>
#include <util/linked_list.h>

static volatile uint32_t timer_jiffies = 0;
static volatile uint16_t timer_hz = 0;
static linked_list_t *timer_wakeup_calls = NULL;

static void timer_set_frequency(uint16_t hz);
static void timer_interrupt_handler(isr_cpu_state_t *state);

void timer_init() {
    timer_set_frequency(100);
    isr_register_listener(PROGRAMMABLE_INTERRUPT_TIMER_INTERRUPT, timer_interrupt_handler);
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

void timer_sleep(uint32_t seconds) {
    uint32_t end = timer_jiffies + (seconds * timer_hz);
    while(timer_jiffies < end);
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
