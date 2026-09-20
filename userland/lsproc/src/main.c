#include <proc.h>
#include <stdio.h>
#include <termio.h>

/** Room for the widest line the columns below can produce. */
#define LSPROC_LINE_LENGTH 80

static const char* state_name(uint32_t state) {
    switch (state) {
        case PROC_STATE_READY:   return "ready";
        case PROC_STATE_RUNNING: return "running";
        case PROC_STATE_WAITING: return "waiting";
        case PROC_STATE_EXITED:  return "exited";
        default:                 return "?";
    }
}

int main(void) {
    procinfo_t info;
    termio_pager_t pager;
    char line[LSPROC_LINE_LENGTH];

    termio_pager_init(&pager);

    // The name is the only column of unpredictable width, so it goes last.
    sprintf(line, "%-6s%-6s%-9s%s\n", "PID", "PPID", "STATE", "NAME");
    termio_pager_puts(&pager, line);

    for (uint32_t index = 0; proc_list(index, &info) == 0; index++) {
        sprintf(line, "%-6d%-6d%-9s%s\n", info.pid, info.parent, state_name(info.state), info.name);

        // The reader has seen enough, the rest of the list is not worth listing.
        if (termio_pager_puts(&pager, line) < 0) {
            break;
        }
    }

    return 0;
}
