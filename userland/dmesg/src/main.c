#include <kmsg.h>
#include <stdio.h>

int main(void) {
    kmsg_entry_t entry;

    for (uint32_t index = 0; kmsg_read(index, &entry) == 0; index++) {
        printf("[%s] %s\n", entry.level, entry.message);
    }

    return 0;
}
