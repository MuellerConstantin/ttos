#include <devio.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>

/*
 * Room for the widest line a device can produce: the columns below, one branch
 * of three characters per level of the tree, and the name.
 */
#define LSDEV_LINE_LENGTH 256

/** Width of a single branch of the tree drawing. */
#define LSDEV_BRANCH_WIDTH 3

static const char* device_type_name(uint16_t type) {
    switch (type) {
        case DEVICE_TYPE_KEYBOARD:   return "KEYBOARD";
        case DEVICE_TYPE_STORAGE:    return "STORAGE";
        case DEVICE_TYPE_VIDEO:      return "VIDEO";
        case DEVICE_TYPE_CONTROLLER: return "CONTROLLER";
        case DEVICE_TYPE_SERIAL:     return "SERIAL";
        case DEVICE_TYPE_TIMER:      return "TIMER";
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
 * Writes the branches leading to a device into a line. Every level above the
 * device gets a vertical bar while that branch still has siblings coming, and
 * blank space once it does not, so a device is always visibly attached to its
 * parent.
 *
 * @param cursor Where in the line to write.
 * @param info The device to draw the branches for.
 * @return The position behind the branches, for the caller to go on writing at.
 */
static char* write_branches(char* cursor, const devinfo_t* info) {
    for (uint8_t level = 1; level < info->depth; level++) {
        strcpy(cursor, info->last_child_mask & (1u << level) ? "   " : "|  ");
        cursor += LSDEV_BRANCH_WIDTH;
    }

    if (info->depth > 0) {
        strcpy(cursor, info->last_child_mask & (1u << info->depth) ? "`- " : "+- ");
        cursor += LSDEV_BRANCH_WIDTH;
    }

    return cursor;
}

int main(void) {
    devinfo_t info;
    termio_pager_t pager;
    char line[LSDEV_LINE_LENGTH];

    termio_pager_init(&pager);

    // The tree is the only column of unpredictable width, so it goes last.
    sprintf(line, "%-8s%-12s%-10s%s\n", "ID", "TYPE", "BUS", "NAME");
    termio_pager_puts(&pager, line);

    for (uint32_t index = 0; devio_list(index, &info) == 0; index++) {
        char* cursor = line;

        cursor += sprintf(cursor, "%-8s%-12s%-10s", info.id, device_type_name(info.type), device_bus_name(info.bus_type));
        cursor = write_branches(cursor, &info);

        sprintf(cursor, "%s\n", info.name);

        // The reader has seen enough, the rest of the tree is not worth walking.
        if (termio_pager_puts(&pager, line) < 0) {
            break;
        }
    }

    return 0;
}
