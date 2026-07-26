#include <volio.h>
#include <stdio.h>

int main(void) {
    volinfo_t info;

    for (uint32_t index = 0; volio_list(index, &info) == 0; index++) {
        printf("%s (%s)\n", info.name, info.id);
    }

    return 0;
}
