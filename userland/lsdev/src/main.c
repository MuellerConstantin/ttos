#include <devio.h>
#include <stdio.h>

int main(void) {
    devinfo_t info;

    for (uint32_t index = 0; devio_list(index, &info) == 0; index++) {
        printf("%s (%s)\n", info.name, info.uuid);
    }

    return 0;
}
