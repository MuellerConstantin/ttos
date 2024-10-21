#include <io/tty.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

static char tty_keycode_to_char(tty_t* tty, uint32_t keycode, bool shifted);

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

    return tty;
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

char tty_getchar(tty_t* tty) {
    static bool shift = false;
    volatile bool waiting = true;
    volatile keyboard_event_t event;

    while(1) {
        // Wait for a key press/release event
        do {
            waiting = !tty->keyboard->driver->available();
        } while(waiting);

        tty->keyboard->driver->dequeue(&event);

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

        char ch = tty_keycode_to_char(tty, event.keycode, shift);

        // Wait for a displayable character
        if(ch) {
            return ch;
        }
    }

    return 0;
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

char* tty_readline(tty_t* tty, bool echo) {
    char *buffer = kmalloc(1);
    size_t buffer_size = 1;
    size_t buffer_index = 0;

    while(true) {
        char ch = tty_getchar(tty);

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

        if(echo) {
            tty_putchar(tty, ch);
        }

        if(buffer_index == buffer_size) {
            buffer_size *= 2;
            buffer = krealloc(buffer, buffer_size);
        }

        buffer[buffer_index] = ch;
        buffer_index++;
    }

    buffer[buffer_index] = '\0';

    return buffer;
}

int tty_printf(tty_t* tty, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int value = tty_vprintf(tty, format, args);
    va_end(args);

    return value;
}

int tty_vprintf(tty_t* tty, const char *format, va_list args) {
    int count = 0;

    while(*format != '\0') {
        if(*format == '%') {
            format++;

            switch(*format) {
                case '%': {
                    tty_putchar(tty, '%');
                    count++;
                    break;
                }
                case 'c': {
                    char ch = va_arg(args, int);
                    tty_putchar(tty, ch);
                    count++;
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, const char*);

                    while(*str != '\0') {
                        tty_putchar(tty, *str);
                        str++;
                        count++;
                    }

                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    itoa(num, num_str, 10);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        tty_putchar(tty, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    tty_putchar(tty, '0');
                    tty_putchar(tty, 'x');

                    itoa(num, num_str, 16);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        tty_putchar(tty, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    char ptr_str[32];

                    itoa((uintptr_t) ptr, ptr_str, 16);

                    for(int i = 0; ptr_str[i] != '\0'; i++) {
                        tty_putchar(tty, ptr_str[i]);
                        count++;
                    }

                    break;
                }
                case 'b': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    tty_putchar(tty, '0');
                    tty_putchar(tty, 'b');

                    itoa(num, num_str, 2);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        tty_putchar(tty, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'f': {
                    double num = va_arg(args, double);
                    char num_str[32];

                    gcvt(num, 2, num_str);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        tty_putchar(tty, num_str[i]);
                        count++;
                    }

                    break;
                }
                default: {
                    break;
                }
            }
        } else {
            tty_putchar(tty, *format);
            count++;
        }

        format++;
    }

    return count;
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
