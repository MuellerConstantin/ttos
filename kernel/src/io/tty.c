#include <io/tty.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <device/keyboard.h>

static tty_t* tty_stdterm = NULL;

static void tty_keyboard_listener(keyboard_event_t* event);
static void tty_render(tty_t* tty, char ch);
static tty_stream_putchar(stream_t* stream, char ch);
static char tty_stream_getchar(stream_t* stream);
static void tty_stream_puts(stream_t* stream, const char* str);
static char* tty_stream_gets(stream_t* stream);

static char tty_keycode_to_char(tty_t* tty, uint32_t keycode, bool shifted);

void tty_set_stdterm(tty_t* tty) {
    tty_stdterm = tty;
}

const tty_t* tty_get_stdterm() {
    return tty_stdterm;
}

tty_t* tty_create(video_device_t* video, keyboard_device_t* keyboard, tty_keyboard_layout_t* layout) {
    if(!video->driver->tm_probe()) {
        return NULL;
    }

    tty_t *tty = (tty_t*) kmalloc(sizeof(tty_t));

    if(tty == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    tty->rows = video->driver->tm.total_rows();
    tty->columns = video->driver->tm.total_columns();
    tty->cursor_x = 0;
    tty->cursor_y = 0;
    tty->fgcolor = TTY_WHITE;
    tty->bgcolor = TTY_BLACK;
    tty->video = video;
    tty->keyboard = keyboard;
    tty->layout = layout;

    tty->input = circular_buffer_create(TTY_BUFFER_SIZE, sizeof(char));

    if (!tty->input) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    tty->output = circular_buffer_create(TTY_BUFFER_SIZE, sizeof(char));

    if (!tty->output) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    if(tty_stdterm == NULL) {
        tty_set_stdterm(tty);
        keyboard->driver->register_listener(tty_keyboard_listener);
    }

    return tty;
}

static void tty_keyboard_listener(keyboard_event_t* event) {
    tty_t* tty = tty_get_stdterm();

    if(tty == NULL) {
        return;
    }

    static bool shift = false;

    if(!event->pressed) {
        if(event->keycode == KEYBOARD_KEYCODE_LEFT_SHIFT || event->keycode == KEYBOARD_KEYCODE_RIGHT_SHIFT) {
            shift = false;
        }

        return;
    }

    if(event->keycode == KEYBOARD_KEYCODE_LEFT_SHIFT || event->keycode == KEYBOARD_KEYCODE_RIGHT_SHIFT) {
        shift = true;
        return;
    }

    if(event->keycode == KEYBOARD_KEYCODE_CAPS_LOCK) {
        shift = !shift;
        return;
    }

    char ch = tty_keycode_to_char(tty, event->keycode, shift);

    // Wait for a displayable character
    if(ch) {
        circular_buffer_enqueue(tty->input, &ch);
    }
}

void tty_flush(tty_t* tty) {
    while(!circular_buffer_empty(tty->output)) {
        char ch = 0;

        circular_buffer_dequeue(tty->output, &ch);

        tty_render(tty, ch);
    }
}

static void tty_render(tty_t* tty, char ch) {
    switch(ch) {
        case '\n':
            tty->cursor_x = 0;
            tty->cursor_y++;
            break;
        case '\r':
            tty->cursor_x = 0;
            break;
        case '\b':
            if(tty->cursor_x > 0) {
                tty->cursor_x--;
            }

            tty->video->driver->tm.write(tty->cursor_y * tty->columns + tty->cursor_x, ' ', tty->fgcolor, tty->bgcolor);
            break;
        case '\t':
            tty->cursor_x = (tty->cursor_x + 8) & ~(8 - 1);
            break;
        default:
            tty->video->driver->tm.write(tty->cursor_y * tty->columns + tty->cursor_x, ch, tty->fgcolor, tty->bgcolor);
            tty->cursor_x++;
            break;
    }

    // Check if end of line has been reached
    if(tty->cursor_x >= tty->columns) {
        tty->cursor_x = 0;
        tty->cursor_y++;
    }

    // Check if end of screen has been reached
    if(tty->cursor_y >= tty->rows) {
        tty->video->driver->tm.scroll(tty->fgcolor, tty->bgcolor);
        tty->cursor_y--;
        tty->video->driver->tm.move_cursor(tty->cursor_y * tty->columns + tty->cursor_x);
    }

    tty->video->driver->tm.move_cursor(tty->cursor_y * tty->columns + tty->cursor_x);
}

stream_t* tty_get_out_stream(tty_t* tty) {
    stream_t *stream = (stream_t*) kmalloc(sizeof(stream_t));

    if(stream == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    stream->putchar = tty_stream_putchar;
    stream->getchar = NULL;
    stream->puts = tty_stream_puts;
    stream->gets = NULL;
    stream->data = tty;

    return stream;
}

stream_t* tty_get_in_stream(tty_t* tty) {
    stream_t *stream = (stream_t*) kmalloc(sizeof(stream_t));

    if(stream == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    stream->putchar = NULL;
    stream->getchar = tty_stream_getchar;
    stream->puts = NULL;
    stream->gets = tty_stream_gets;
    stream->data = tty;

    return stream;
}

stream_t* tty_get_err_stream(tty_t* tty) {
    return tty_get_out_stream(tty);
}

static tty_stream_putchar(stream_t* stream, char ch) {
    tty_putchar((tty_t*) stream->data, ch);
}

static char tty_stream_getchar(stream_t* stream) {
    return tty_getchar((tty_t*) stream->data);
}

static void tty_stream_puts(stream_t* stream, const char* str) {
    tty_puts((tty_t*) stream->data, str);
}

static char* tty_stream_gets(stream_t* stream) {
    return tty_gets((tty_t*) stream->data);
}

void tty_clear(tty_t* tty) {
    for(size_t y = 0; y < tty->rows; y++) {
        for(size_t x = 0; x < tty->columns; x++) {
            tty->video->driver->tm.write(y * tty->columns + x, ' ', tty->fgcolor, tty->bgcolor);
        }
    }

    tty->cursor_x = 0;
    tty->cursor_y = 0;
    tty->video->driver->tm.move_cursor(tty->cursor_y * tty->columns + tty->cursor_x);
}

void tty_putchar(tty_t* tty, char ch) {
    circular_buffer_enqueue(tty->output, &ch);
    tty_flush(tty);
}

char tty_getchar(tty_t* tty) {
    char ch;

    if(circular_buffer_empty(tty->input)) {
        return -1;
    }

    circular_buffer_dequeue(tty->input, &ch);

    return ch;
}

static char tty_keycode_to_char(tty_t* tty, uint32_t keycode, bool shifted) {
    for(size_t index = 0; index < tty->layout->keymap_size; index++) {
        if(tty->layout->keymap[index].keycode == keycode) {
            if(shifted) {
                return tty->layout->keymap[index].shifted;
            } else {
                return tty->layout->keymap[index].normal;
            }
        }
    }

    return 0;
}

void tty_puts(tty_t* tty, const char* str) {
    for(size_t i = 0; str[i] != '\0'; i++) {
        tty_putchar(tty, str[i]);
    }
}

char* tty_gets(tty_t* tty) {
    char *buffer = kmalloc(1);
    size_t buffer_size = 1;
    size_t buffer_index = 0;

    while(true) {
        char ch;
        while((ch = tty_getchar(tty)) == -1);

        if(ch == '\n') {
            tty_putchar(tty, ch);
            break;
        }

        if(ch == '\b') {
            if(buffer_index == 0) {
                continue;
            }

            buffer_index--;

            tty_putchar(tty, ch);

            continue;
        }

        if(buffer_index == buffer_size) {
            buffer_size *= 2;
            buffer = krealloc(buffer, buffer_size);
        }

        buffer[buffer_index] = ch;
        buffer_index++;
        tty_putchar(tty, ch);
    }

    buffer[buffer_index] = '\0';

    return buffer;
}

void tty_set_fgcolor(tty_t* tty, uint8_t fgcolor) {
    tty->fgcolor = fgcolor;
}

void tty_set_bgcolor(tty_t* tty, uint8_t bgcolor) {
    tty->bgcolor = bgcolor;
}

void tty_disable_cursor(tty_t* tty) {
    tty->video->driver->tm.disable_cursor();
}

void tty_enable_cursor(tty_t* tty) {
    tty->video->driver->tm.enable_cursor(0, 15);
}
