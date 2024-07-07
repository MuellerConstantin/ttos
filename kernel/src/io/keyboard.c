#include <io/keyboard.h>

static keyboard_event_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static size_t keyboard_buffer_head = 0;
static size_t keyboard_buffer_tail = 0;
static bool keyboard_buffer_full = false;

void keyboard_buffer_enqueue(keyboard_event_t event) {
    keyboard_buffer[keyboard_buffer_head] = event;

    if (keyboard_buffer_full) {
        keyboard_buffer_tail = (keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    }

    keyboard_buffer_head = (keyboard_buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer_full = (bool) keyboard_buffer_head == keyboard_buffer_tail;
}

keyboard_event_t keyboard_buffer_dequeue() {
    if (keyboard_buffer_tail == keyboard_buffer_head && !keyboard_buffer_full) {
        return (keyboard_event_t) { 0 };
    }

    keyboard_event_t event = keyboard_buffer[keyboard_buffer_tail];
    keyboard_buffer_tail = (keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer_full = false;

    return event;
}

bool keyboard_buffer_available() {
    return keyboard_buffer_tail != keyboard_buffer_head || keyboard_buffer_full;
}
