#include <io/shell.h>
#include <string.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <memory/pmm.h>
#include <arch/i386/acpi.h>
#include <arch/i386/isr.h>
#include <system/kmessage.h>
#include <device/device.h>
#include <device/volume.h>
#include <fs/mount.h>
#include <io/dir.h>
#include <uuid.h>

static void shell_process_instruction(shell_t *shell, char *instruction);
static void shell_help(shell_t *shell, size_t argc, const char *argv[]);
static void shell_clear(shell_t *shell, size_t argc, const char *argv[]);
static void shell_echo(shell_t *shell, size_t argc, const char *argv[]);
static void shell_memory_usage(shell_t *shell, size_t argc, const char *argv[]);
static void shell_memory_map(shell_t *shell, size_t argc, const char *argv[]);
static void shell_poweroff(shell_t *shell, size_t argc, const char *argv[]);
static void shell_dmesg(shell_t *shell, size_t argc, const char *argv[]);
static void shell_lsdev(shell_t *shell, size_t argc, const char *argv[]);
static void shell_lsvol(shell_t *shell, size_t argc, const char *argv[]);
static void shell_lsmnt(shell_t *shell, size_t argc, const char *argv[]);
static void shell_mount(shell_t *shell, size_t argc, const char *argv[]);
static void shell_unmount(shell_t *shell, size_t argc, const char *argv[]);
static void shell_lsdir(shell_t *shell, size_t argc, const char *argv[]);
static void shell_kheap_usage(shell_t *shell, size_t argc, const char *argv[]);

shell_t* shell_create(tty_t *tty0) {
    shell_t *shell = kmalloc(sizeof(shell_t));

    if(shell == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    shell->tty = tty0;

    return shell;
}

void shell_execute(shell_t *shell) {
    while(1) {
        tty_putchar(shell->tty, '>');
        tty_putchar(shell->tty, ' ');

        char *instruction = tty_readline(shell->tty, true);

        shell_process_instruction(shell, instruction);

        kfree(instruction);
    }
}

static void shell_process_instruction(shell_t *shell, char *instruction) {
    char* instruction_copy = kmalloc(strlen(instruction) + 1);

    if(instruction_copy == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    strcpy(instruction_copy, instruction);

    char *command = strsep(&instruction_copy, " ");

    size_t argc = 1;
    char **argv = kmalloc(sizeof(char*));

    if(argv == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    argv[0] = command;

    char *argument;
    char *rest = instruction_copy;
    
    while (rest != NULL) {
        // Check for quoted arguments
        if (*rest == '"') {
            rest++;
            argument = strsep(&rest, "\"");

            if(*argument != '\0') {
                argc++;
                argv = krealloc(argv, argc * sizeof(char*));
                argv[argc - 1] = argument;
            }
        } else {
            argument = strsep(&rest, " ");

            if (*argument != '\0') {
                argc++;
                argv = krealloc(argv, argc * sizeof(char*));
                argv[argc - 1] = argument;
            }
        }

        // Skip additional spaces
        while (rest != NULL && *rest == ' ') {
            rest++;
        }
    }

    if(strcmp(command, "help") == 0) {
        shell_help(shell, argc, argv);
    } else if(strcmp(command, "clear") == 0) {
        shell_clear(shell, argc, argv);
    } else if(strcmp(command, "echo") == 0) {
        shell_echo(shell, argc, argv);
    } else if(strcmp(command, "memusage") == 0) {
        shell_memory_usage(shell, argc, argv);
    } else if(strcmp(command, "kheapusage") == 0) {
        shell_kheap_usage(shell, argc, argv);
    } else if(strcmp(command, "memmap") == 0) {
        shell_memory_map(shell, argc, argv);
    } else if(strcmp(command, "poweroff") == 0) {
        shell_poweroff(shell, argc, argv);
    } else if(strcmp(command, "dmesg") == 0) {
        shell_dmesg(shell, argc, argv);
    } else if(strcmp(command, "lsdev") == 0) {
        shell_lsdev(shell, argc, argv);
    } else if(strcmp(command, "lsvol") == 0) {
        shell_lsvol(shell, argc, argv);
    } else if(strcmp(command, "lsmnt") == 0) {
        shell_lsmnt(shell, argc, argv);
    } else if(strcmp(command, "mount") == 0) {
        shell_mount(shell, argc, argv);
    } else if(strcmp(command, "unmount") == 0) {
        shell_unmount(shell, argc, argv);
    } else if(strcmp(command, "lsdir") == 0) {
        shell_lsdir(shell, argc, argv);
    } else {
        tty_printf(shell->tty, "Unknown command: %s\n", command);
    }

    kfree(argv);
    kfree(instruction_copy);
}

static void shell_help(shell_t *shell, size_t argc, const char *argv[]) {
    const char* help_message = "Available commands:\n\n"
        "help - Display this help message\n"
        "clear - Clear the screen\n"
        "echo <text> - Echo the text\n"
        "memusage - Display memory usage\n"
        "kheapusage - Display kernel heap usage\n"
        "memmap - Display memory map\n"
        "poweroff - Power off the system\n"
        "dmesg - Display kernel messages\n"
        "lsdev - List available devices\n"
        "lsvol - List available volumes\n"
        "lsmnt - List available mount points\n"
        "mount <drive> <volume> - Mount a volume to a drive\n"
        "unmount <drive> - Unmount a volume from a drive\n"
        "lsdir <path> - List directory contents\n";

    tty_paging(shell->tty, help_message);
}

static void shell_clear(shell_t *shell, size_t argc, const char *argv[]) {
    tty_clear(shell->tty);
}

static void shell_echo(shell_t *shell, size_t argc, const char *argv[]) {
    if(argc < 2) {
        tty_printf(shell->tty, "Usage: echo <text>\n");
        return;
    }

    tty_printf(shell->tty, "%s\n", argv[1]);
}

static void shell_memory_usage(shell_t *shell, size_t argc, const char *argv[]) {
    size_t total_memory = pmm_get_total_memory_size();
    size_t free_memory = pmm_get_available_memory_size();

    double total_memory_mb = total_memory / 1024 / 1024;
    double free_memory_mb = free_memory / 1024 / 1024;
    double used_memory_mb = total_memory_mb - free_memory_mb;
    double used_memory_percentage = (used_memory_mb / total_memory_mb) * 100;

    tty_printf(shell->tty, "%f MB / %f MB (%f%%) used\n", used_memory_mb, total_memory_mb, used_memory_percentage);
}

static void shell_kheap_usage(shell_t *shell, size_t argc, const char *argv[]) {
    size_t total_memory = kheap_get_total_memory_size();
    size_t free_memory = kheap_get_available_memory_size();

    double total_memory_mb = total_memory / 1024 / 1024;
    double free_memory_mb = free_memory / 1024 / 1024;
    double used_memory_mb = total_memory_mb - free_memory_mb;
    double used_memory_percentage = (used_memory_mb / total_memory_mb) * 100;

    tty_printf(shell->tty, "%f MB / %f MB (%f%%) used\n", used_memory_mb, total_memory_mb, used_memory_percentage);
}

static void shell_memory_map(shell_t *shell, size_t argc, const char *argv[]) {
    const linked_list_t* pmm_memory_regions = pmm_get_memory_regions();

    linked_list_foreach(pmm_memory_regions, node) {
        pmm_memory_region_t* region = (pmm_memory_region_t*) node->data;

        tty_printf(shell->tty, "Memory region: %x - %x (%d bytes) Type: %x\n", region->base, region->base + region->length - 1, region->length, region->type);
    }
}

static void shell_poweroff(shell_t *shell, size_t argc, const char *argv[]) {
    acpi_poweroff();

    tty_printf(shell->tty, "It is now safe to turn off your computer...\n");
    tty_disable_cursor(shell->tty);

    isr_cli();
    
    while(1);
}

static void shell_dmesg(shell_t *shell, size_t argc, const char *argv[]) {
    const linked_list_t* messages = kmessage_get_messages();
    
    char* message_buffer = kmalloc(1024);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    linked_list_foreach(messages, node) {
        kmessage_message_t* message = (kmessage_message_t*) node->data;

        message_buffer = krealloc(message_buffer, strlen(message_buffer) + strlen(message->level) + strlen(message->message) + 5);

        if(message_buffer == NULL) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
        }

        strcat(message_buffer, "[");
        strcat(message_buffer, message->level);
        strcat(message_buffer, "] ");
        strcat(message_buffer, message->message);
        strcat(message_buffer, "\n");
    }

    tty_paging(shell->tty, message_buffer);

    kfree(message_buffer);
}

static void shell_lsdev_append_callback(generic_tree_node_t* node, void* data) {
    device_t* device = (device_t*) node->data;
    char** message_buffer_pointer = (char**) data;
    char* message_buffer = *message_buffer_pointer;

    message_buffer = krealloc(message_buffer, strlen(message_buffer) + strlen(device->name) + 36 + 4);

    char uuid_buffer[37];
    uuid_v4_to_string(&device->id, uuid_buffer);

    strcat(message_buffer, device->name);
    strcat(message_buffer, " (");
    strcat(message_buffer, uuid_buffer);
    strcat(message_buffer, ")\n");

    *message_buffer_pointer = message_buffer;
}

static void shell_lsdev(shell_t *shell, size_t argc, const char *argv[]) {
    const generic_tree_t* devices = device_get_all();

    char* message_buffer = kmalloc(1024);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    generic_tree_foreach(devices, shell_lsdev_append_callback, &message_buffer);

    tty_paging(shell->tty, message_buffer);

    kfree(message_buffer);
}

static void shell_lsvol(shell_t *shell, size_t argc, const char *argv[]) {
    const linked_list_t* volumes = volume_get_all();

    char* message_buffer = kmalloc(512);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    linked_list_foreach(volumes, node) {
        volume_t* volume = (volume_t*) node->data;

        message_buffer = krealloc(message_buffer, strlen(message_buffer) + strlen(volume->name) + 36 + 4);

        char uuid_buffer[37];
        uuid_v4_to_string(&volume->id, uuid_buffer);

        strcat(message_buffer, volume->name);
        strcat(message_buffer, " (");
        strcat(message_buffer, uuid_buffer);
        strcat(message_buffer, ")\n");
    }

    tty_paging(shell->tty, message_buffer);

    kfree(message_buffer);
}

static void shell_lsmnt(shell_t *shell, size_t argc, const char *argv[]) {
    char* message_buffer = kmalloc(512);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    for(char drive = DRIVE_A; drive <= DRIVE_Z; drive++) {
        if(mnt_get_drive(drive) != NULL) {
            message_buffer = krealloc(message_buffer, strlen(message_buffer) + 3);

            message_buffer[strlen(message_buffer)] = drive;
            strcat(message_buffer, ":\n");
        }
    }

    tty_paging(shell->tty, message_buffer);

    kfree(message_buffer);
}

static void shell_mount(shell_t *shell, size_t argc, const char *argv[]) {
    if(argc < 3) {
        tty_printf(shell->tty, "Usage: mount <drive> <volume>\n");
        return;
    }

    char drive = argv[1][0];
    volume_t* volume = volume_find_by_name(argv[2]);

    if(volume == NULL) {
        tty_printf(shell->tty, "Volume not found\n");
        return;
    }

    if(mnt_get_drive(drive) != NULL) {
        tty_printf(shell->tty, "Drive already mounted\n");
        return;
    }

    if(mnt_volume_mount(drive, volume) != 0) {
        tty_printf(shell->tty, "Failed to mount volume, file system may not be supported\n");
        return;
    }

    tty_printf(shell->tty, "Volume mounted\n");
}

static void shell_unmount(shell_t *shell, size_t argc, const char *argv[]) {
    if(argc < 2) {
        tty_printf(shell->tty, "Usage: unmount <drive>\n");
        return;
    }

    char drive = argv[1][0];

    if(mnt_get_drive(drive) == NULL) {
        tty_printf(shell->tty, "Drive not mounted\n");
        return;
    }

    if(mnt_volume_unmount(drive) != 0) {
        tty_printf(shell->tty, "Failed to unmount volume\n");
        return;
    }

    tty_printf(shell->tty, "Volume unmounted\n");
}

static void shell_lsdir(shell_t *shell, size_t argc, const char *argv[]) {
    if(argc < 2) {
        tty_printf(shell->tty, "Usage: lsdir <path>\n");
        return;
    }

    int32_t dd = dir_open(argv[1]);

    if(dd < 0) {
        tty_printf(shell->tty, "Invalid path\n");
        return;
    }

    char* message_buffer = kmalloc(64);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    const dir_dirent_t* dirent = NULL;

    do {
        dirent = dir_read(dd);

        if(dirent != NULL) {
            message_buffer = krealloc(message_buffer, strlen(message_buffer) + strlen(dirent->name) + 1);

            strcat(message_buffer, dirent->name);
            strcat(message_buffer, "\n");

            kfree(dirent);
        }
    } while(dirent != NULL);

    dir_close(dd);

    tty_paging(shell->tty, message_buffer);

    kfree(message_buffer);
}
