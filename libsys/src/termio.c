#include <termio.h>
#include <syscall.h>
#include <fsio.h>
#include <sysinfo.h>

int termio_set_foreground(pid_t pid) {
    return syscall1(SYSCALL_SET_FOREGROUND, (uint32_t) pid);
}

/** Dimensions to fall back to when the terminal does not report its own. */
#define TERMIO_DEFAULT_ROWS 25
#define TERMIO_DEFAULT_COLUMNS 80

/** Keys the prompt understands. */
#define TERMIO_KEY_QUIT 'q'
#define TERMIO_KEY_NEXT 'n'
#define TERMIO_KEY_SPACE ' '
#define TERMIO_KEY_RETURN '\n'

static void termio_pager_prompt(termio_pager_t* pager);

void termio_pager_init(termio_pager_t* pager) {
    terminfo_t terminal;

    pager->rows = TERMIO_DEFAULT_ROWS;
    pager->columns = TERMIO_DEFAULT_COLUMNS;

    /*
     * A terminal one row high leaves no room for the output next to the
     * prompt, so an implausible answer is treated like no answer at all.
     */
    if(sysinfo_get_terminfo(&terminal) == 0 && terminal.rows > 1 && terminal.cols > 0) {
        pager->rows = terminal.rows;
        pager->columns = terminal.cols;
    }

    pager->used_rows = 0;
    pager->used_columns = 0;
    pager->page_full = 0;
    pager->stopped = 0;
}

int32_t termio_pager_putchar(termio_pager_t* pager, char ch) {
    if(pager->stopped) {
        return -1;
    }

    /*
     * The wait happens before the first character of the next screenful rather
     * than after the last one of this one: output that ends exactly on the
     * screen boundary is complete, and holding it would ask the reader to
     * confirm a page that never comes.
     */
    if(pager->page_full) {
        termio_pager_prompt(pager);

        if(pager->stopped) {
            return -1;
        }
    }

    if(fsio_write(FSIO_STDOUT, &ch, 1) < 0) {
        return -1;
    }

    /*
     * The terminal wraps a line that runs past its width onto the next one, so
     * a long line costs as many rows as it wraps.
     */
    if(ch == '\n') {
        pager->used_rows++;
        pager->used_columns = 0;
    } else {
        pager->used_columns++;

        if(pager->used_columns >= pager->columns) {
            pager->used_rows++;
            pager->used_columns = 0;
        }
    }

    // The bottom row belongs to the prompt, so the page is full one row early.
    if(pager->used_rows >= pager->rows - 1) {
        pager->page_full = 1;
    }

    return 0;
}

int32_t termio_pager_puts(termio_pager_t* pager, const char* str) {
    while(*str != '\0') {
        if(termio_pager_putchar(pager, *str) < 0) {
            return -1;
        }

        str++;
    }

    return 0;
}

int32_t termio_pager_stopped(const termio_pager_t* pager) {
    return pager->stopped ? 1 : 0;
}

/**
 * Holds the full screen until the reader has seen it. Keys other than the ones
 * below are ignored rather than taken as "go on", so that the escape sequence
 * of an arrow key does not skip a page.
 */
static void termio_pager_prompt(termio_pager_t* pager) {
    char ch;

    fsio_write(FSIO_STDOUT, ":", 1);

    for(;;) {
        int32_t bytes_read = fsio_read(FSIO_STDIN, &ch, 1);

        // Losing the input is the one thing the reader cannot recover from.
        if(bytes_read < 0) {
            pager->stopped = 1;
            break;
        }

        if(bytes_read == 0) {
            continue;
        }

        if(ch == TERMIO_KEY_QUIT) {
            pager->stopped = 1;
            break;
        }

        if(ch == TERMIO_KEY_NEXT || ch == TERMIO_KEY_SPACE || ch == TERMIO_KEY_RETURN) {
            break;
        }
    }

    // Take the prompt back off the screen, the page below it starts here.
    fsio_write(FSIO_STDOUT, "\b \b", 3);

    pager->used_rows = 0;
    pager->used_columns = 0;
    pager->page_full = 0;
}
