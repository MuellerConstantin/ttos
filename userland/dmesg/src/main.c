#include <kmsg.h>
#include <stdio.h>
#include <termio.h>

/** Room for the longest line an entry can produce, plus its brackets. */
#define DMESG_LINE_LENGTH 288

int main(void) {
    kmsg_entry_t entry;
    termio_pager_t pager;
    char line[DMESG_LINE_LENGTH];

    termio_pager_init(&pager);

    for (uint32_t index = 0; kmsg_read(index, &entry) == 0; index++) {
        sprintf(line, "[%s] %s\n", entry.level, entry.message);

        // The reader has seen enough, the rest of the log is not worth printing.
        if (termio_pager_puts(&pager, line) < 0) {
            break;
        }
    }

    return 0;
}
