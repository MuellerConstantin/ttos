#include <drivers/input/keyboard.h>
#include <sys/kpanic.h>
#include <ds/circular_buffer.h>
#include <drivers/serial/uart/16550.h>

static circular_buffer_t* keyboard_buffer;
keyboard_layout_t keyboard_layout_current;

extern keyboard_layout_t keyboard_layout_de_DE;

static char keyboard_keycode_to_char(uint32_t keycode, bool shifted);

void keyboard_init() {
    keyboard_buffer = circular_buffer_create(KEYBOARD_BUFFER_SIZE, sizeof(keyboard_event_t));

    if (!keyboard_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    keyboard_layout_current = keyboard_layout_de_DE;
}

void keyboard_enqueue(keyboard_event_t* event) {
    circular_buffer_enqueue(keyboard_buffer, event);
}

void keyboard_dequeue(keyboard_event_t* event) {
    circular_buffer_dequeue(keyboard_buffer, event);
}

bool keyboard_available() {
    return !circular_buffer_empty(keyboard_buffer);
}

char keyboard_getchar() {
    static bool shift = false;
    volatile bool waiting = true;
    volatile keyboard_event_t event;

    while(1) {
        // Wait for a key press/release event
        do {
            waiting = !keyboard_available();
        } while(waiting);

        keyboard_dequeue(&event);

        if(!event.pressed) {
            if(event.keycode == KEYBOARD_KEYCODE_LEFT_SHIFT || event.keycode == KEYBOARD_KEYCODE_RIGHT_SHIFT) {
                shift = false;
            }

            continue;
        }

        if(event.keycode == KEYBOARD_KEYCODE_LEFT_SHIFT || event.keycode == KEYBOARD_KEYCODE_RIGHT_SHIFT) {
            shift = true;
            continue;
        }

        if(event.keycode == KEYBOARD_KEYCODE_CAPS_LOCK) {
            shift = !shift;
            continue;
        }

        char ch = keyboard_keycode_to_char(event.keycode, shift);

        // Wait for a displayable character
        if(ch) {
            return ch;
        }
    }

    return 0;
}

static char keyboard_keycode_to_char(uint32_t keycode, bool shifted) {
    for(size_t index = 0; index < keyboard_layout_current.keymap_size; index++) {
        if(keyboard_layout_current.keymap[index].keycode == keycode) {
            if(shifted) {
                return keyboard_layout_current.keymap[index].shifted;
            } else {
                return keyboard_layout_current.keymap[index].normal;
            }
        }
    }

    return 0;
}
