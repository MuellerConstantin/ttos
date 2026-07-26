#include <power.h>
#include <stdio.h>

int main(void) {
    power_off();

    // Only reached when the power off could not be performed.
    puts("poweroff: failed to power off the system\n");

    return 1;
}
