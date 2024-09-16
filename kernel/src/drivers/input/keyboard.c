#include <drivers/input/keyboard.h>
#include <sys/kpanic.h>
#include <ds/circular_buffer.h>

static circular_buffer_t* keyboard_buffer;

void keyboard_init() {
    keyboard_buffer = circular_buffer_create(KEYBOARD_BUFFER_SIZE, sizeof(keyboard_event_t));

    if (!keyboard_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }
}

void keyboard_enqueue(keyboard_event_t* event) {
    circular_buffer_enqueue(keyboard_buffer, event);
}

void keyboard_dequeue(keyboard_event_t* event) {
    return circular_buffer_dequeue(keyboard_buffer, event);
}

bool keyboard_available() {
    return !circular_buffer_empty(keyboard_buffer);
}
