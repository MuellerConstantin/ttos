#include <devio.h>
#include <stdio.h>

static const char* device_type_name(uint16_t type) {
    switch (type) {
        case DEVICE_TYPE_KEYBOARD:   return "KEYBOARD";
        case DEVICE_TYPE_STORAGE:    return "STORAGE";
        case DEVICE_TYPE_VIDEO:      return "VIDEO";
        case DEVICE_TYPE_CONTROLLER: return "CONTROLLER";
        case DEVICE_TYPE_SERIAL:     return "SERIAL";
        case DEVICE_TYPE_RESERVED:   return "RESERVED";
        case DEVICE_TYPE_UNKNOWN:    return "UNKNOWN";
        default:                     return "?";
    }
}

static const char* device_bus_name(uint8_t bus_type) {
    switch (bus_type) {
        case DEVICE_BUS_TYPE_PLATFORM: return "PLATFORM";
        case DEVICE_BUS_TYPE_ISA:      return "ISA";
        case DEVICE_BUS_TYPE_PCI:      return "PCI";
        case DEVICE_BUS_TYPE_USB:      return "USB";
        case DEVICE_BUS_TYPE_ATA:      return "ATA";
        case DEVICE_BUS_TYPE_PS2:      return "PS/2";
        case DEVICE_BUS_TYPE_RESERVED: return "-";
        default:                       return "?";
    }
}

/**
 * Draws the branches leading to a device. Every level above the device gets a
 * vertical bar while that branch still has siblings coming, and blank space
 * once it does not, so a device is always visibly attached to its parent.
 */
static void print_branches(const devinfo_t* info) {
    for (uint8_t level = 1; level < info->depth; level++) {
        puts(info->last_child_mask & (1u << level) ? "   " : "|  ");
    }

    if (info->depth > 0) {
        puts(info->last_child_mask & (1u << info->depth) ? "`- " : "+- ");
    }
}

int main(void) {
    devinfo_t info;

    // The tree is the only column of unpredictable width, so it goes last.
    printf("%-8s%-12s%-10s%s\n", "ID", "TYPE", "BUS", "NAME");

    for (uint32_t index = 0; devio_list(index, &info) == 0; index++) {
        printf("%-8s%-12s%-10s", info.id, device_type_name(info.type), device_bus_name(info.bus_type));

        print_branches(&info);

        printf("%s\n", info.name);
    }

    return 0;
}
