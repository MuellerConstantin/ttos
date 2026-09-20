#ifndef _LIBSYS_TERMIO_H
#define _LIBSYS_TERMIO_H

#include <stdint.h>
#include <stddef.h>
#include <ttos/syscall.h>

/**
 * Hands the terminal's foreground to a process. The foreground process is the
 * only one that may read from the terminal and the one Ctrl+C terminates.
 * Only the current foreground process may hand it over, and only to itself
 * or one of its children; while no process holds the foreground, anyone may
 * claim it and becomes the terminal's leader, which Ctrl+C leaves alone.
 *
 * @param pid The PID of the new foreground process, or 0 for the caller
 * @return 0 on success, or -1 if the caller may not hand over the foreground
 *         or the PID is neither the caller nor a live child of it
 */
int termio_set_foreground(pid_t pid);

typedef struct termio_pager termio_pager_t;

/**
 * Keeps track of how much of the terminal the output of a program has filled.
 *
 * A program that writes more than one screenful pushes its first lines off the
 * top of the terminal, where they are gone for good: there is no scrollback to
 * go looking for them in. Writing through a pager instead stops at the end of
 * every screenful and lets the reader decide when to go on.
 *
 * The fields are an implementation detail. Callers only ever hand the struct
 * to the functions below.
 */
struct termio_pager {
    /** Dimensions of the terminal, as reported when the pager was set up. */
    uint32_t rows;
    uint32_t columns;

    /** How much of the current screenful is already taken. */
    uint32_t used_rows;
    uint32_t used_columns;

    /** Set when the screen is full and the next character has to wait for the reader. */
    int32_t page_full;

    /** Set once the reader has asked to stop, which makes every write a no-op. */
    int32_t stopped;
};

/**
 * Prepares a pager for writing to the terminal. The terminal dimensions are
 * read once here, so a pager is good for one run of output and is set up again
 * for the next.
 *
 * @param pager The pager to prepare.
 */
void termio_pager_init(termio_pager_t* pager);

/**
 * Writes a single character through the pager, stopping for the reader
 * whenever the screen is full.
 *
 * @param pager The pager to write through.
 * @param ch The character to write.
 * @return 0 on success or -1 once the reader has asked to stop.
 */
int32_t termio_pager_putchar(termio_pager_t* pager, char ch);

/**
 * Writes a string through the pager, stopping for the reader whenever the
 * screen is full. No newline is appended, matching puts.
 *
 * @param pager The pager to write through.
 * @param str The string to write.
 * @return 0 on success or -1 once the reader has asked to stop.
 */
int32_t termio_pager_puts(termio_pager_t* pager, const char* str);

/**
 * Reports whether the reader has asked to stop. Programs that produce their
 * output at some cost can check this to give up early instead of formatting
 * lines nobody will see.
 *
 * @param pager The pager to check.
 * @return 1 when the reader has asked to stop, 0 otherwise.
 */
int32_t termio_pager_stopped(const termio_pager_t* pager);

#endif // _LIBSYS_TERMIO_H
