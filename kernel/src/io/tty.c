#include <io/tty.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>

static char tty_keycode_to_char(tty_t* tty0, uint32_t keycode, bool shifted);

tty_t* tty_create(video_device_t* video, keyboard_device_t* keyboard, tty_keyboard_layout_t* layout) {
    if(!video->driver->tm_probe()) {
        return NULL;
    }

    tty_t *tty0 = (tty_t*) kmalloc(sizeof(tty_t));

    if(tty0 == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    tty0->rows = video->driver->tm.total_rows();
    tty0->columns = video->driver->tm.total_columns();
    tty0->cursor_x = 0;
    tty0->cursor_y = 0;
    tty0->fgcolor = TTY_WHITE;
    tty0->bgcolor = TTY_BLACK;
    tty0->video = video;
    tty0->keyboard = keyboard;
    tty0->layout = layout;

    return tty0;
}

void tty_putchar(tty_t* tty0, char ch) {
    switch(ch) {
        case '\n':
            tty0->cursor_x = 0;
            tty0->cursor_y++;
            break;
        case '\r':
            tty0->cursor_x = 0;
            break;
        case '\b':
            if(tty0->cursor_x > 0) {
                tty0->cursor_x--;
            }

            tty0->video->driver->tm.write(tty0->cursor_y * tty0->columns + tty0->cursor_x, ' ', tty0->fgcolor, tty0->bgcolor);
            break;
        case '\t':
            tty0->cursor_x = (tty0->cursor_x + 8) & ~(8 - 1);
            break;
        default:
            tty0->video->driver->tm.write(tty0->cursor_y * tty0->columns + tty0->cursor_x, ch, tty0->fgcolor, tty0->bgcolor);
            tty0->cursor_x++;
            break;
    }

    // Check if end of line has been reached
    if(tty0->cursor_x >= tty0->columns) {
        tty0->cursor_x = 0;
        tty0->cursor_y++;
    }

    // Check if end of screen has been reached
    if(tty0->cursor_y >= tty0->rows) {
        tty0->video->driver->tm.scroll(tty0->fgcolor, tty0->bgcolor);
        tty0->cursor_y--;
        tty0->video->driver->tm.move_cursor(tty0->cursor_y * tty0->columns + tty0->cursor_x);
    }

    vga_tm_move_cursor(tty0->cursor_y * tty0->columns + tty0->cursor_x);
}

char tty_getchar(tty_t* tty0) {
    static bool shift = false;
    volatile bool waiting = true;
    volatile keyboard_event_t event;

    while(1) {
        // Wait for a key press/release event
        do {
            waiting = !tty0->keyboard->driver->available();
        } while(waiting);

        tty0->keyboard->driver->dequeue(&event);

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

        char ch = tty_keycode_to_char(tty0, event.keycode, shift);

        // Wait for a displayable character
        if(ch) {
            return ch;
        }
    }

    return 0;
}

static char tty_keycode_to_char(tty_t* tty0, uint32_t keycode, bool shifted) {
    for(size_t index = 0; index < tty0->layout->keymap_size; index++) {
        if(tty0->layout->keymap[index].keycode == keycode) {
            if(shifted) {
                return tty0->layout->keymap[index].shifted;
            } else {
                return tty0->layout->keymap[index].normal;
            }
        }
    }

    return 0;
}

char* tty_readline(tty_t* tty0, bool echo) {
    char *buffer = kmalloc(1);
    size_t buffer_size = 1;
    size_t buffer_index = 0;

    while(true) {
        char ch = tty_getchar(tty0);

        if(ch == '\n') {
            tty_putchar(tty0, ch);
            break;
        }

        if(ch == '\b') {
            if(buffer_index == 0) {
                continue;
            }

            buffer_index--;
            tty_putchar(tty0, ch);

            continue;
        }

        if(echo) {
            tty_putchar(tty0, ch);
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

int tty_printf(tty_t* tty0, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int value = tty_vprintf(tty0, format, args);
    va_end(args);

    return value;
}

int tty_vprintf(tty_t* tty0, const char *format, va_list args) {
    int count = 0;

    while(*format != '\0') {
        if(*format == '%') {
            format++;

            switch(*format) {
                case 'c': {
                    char ch = va_arg(args, int);
                    tty_putchar(tty0, ch);
                    count++;
                    break;
                }
                case 's': {
                    const char *str = va_arg(args, const char*);

                    while(*str != '\0') {
                        tty_putchar(tty0, *str);
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
                        tty_putchar(tty0, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'x': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    tty_putchar(tty0, '0');
                    tty_putchar(tty0, 'x');

                    itoa(num, num_str, 16);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        tty_putchar(tty0, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'p': {
                    void *ptr = va_arg(args, void*);
                    char ptr_str[32];

                    itoa((uintptr_t) ptr, ptr_str, 16);

                    for(int i = 0; ptr_str[i] != '\0'; i++) {
                        tty_putchar(tty0, ptr_str[i]);
                        count++;
                    }

                    break;
                }
                case 'b': {
                    int num = va_arg(args, int);
                    char num_str[32];

                    tty_putchar(tty0, '0');
                    tty_putchar(tty0, 'b');

                    itoa(num, num_str, 2);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        tty_putchar(tty0, num_str[i]);
                        count++;
                    }

                    break;
                }
                case 'f': {
                    double num = va_arg(args, double);
                    char num_str[32];

                    gcvt(num, 4, num_str);

                    for(int i = 0; num_str[i] != '\0'; i++) {
                        tty_putchar(tty0, num_str[i]);
                        count++;
                    }

                    break;
                }
                default: {
                    break;
                }
            }
        } else {
            tty_putchar(tty0, *format);
            count++;
        }

        format++;
    }

    return count;
}

void tty_set_fgcolor(tty_t* tty0, uint8_t fgcolor) {
    tty0->fgcolor = fgcolor;
}

void tty_set_bgcolor(tty_t* tty0, uint8_t bgcolor) {
    tty0->bgcolor = bgcolor;
}
