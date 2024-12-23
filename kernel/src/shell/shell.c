#include <shell/shell.h>
#include <util/string.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <system/process.h>
#include <memory/pmm.h>
#include <arch/i386/acpi.h>
#include <arch/i386/isr.h>
#include <system/kmessage.h>
#include <system/timer.h>
#include <device/device.h>
#include <device/volume.h>
#include <fs/mount.h>
#include <io/file.h>
#include <io/dir.h>
#include <util/uuid.h>
#include <io/tty.h>

static stream_t* in_stream = NULL;
static stream_t* out_stream = NULL;
static stream_t* err_stream = NULL;

static void shell_display_banner();
static void shell_process_instruction(char *instruction);
static void shell_paging(const char* buffer);
static void shell_help(size_t argc, const char *argv[]);
static void shell_clear(size_t argc, const char *argv[]);
static void shell_echo(size_t argc, const char *argv[]);
static void shell_memory_usage(size_t argc, const char *argv[]);
static void shell_memory_map(size_t argc, const char *argv[]);
static void shell_poweroff(size_t argc, const char *argv[]);
static void shell_dmesg(size_t argc, const char *argv[]);
static void shell_lsdev(size_t argc, const char *argv[]);
static void shell_lsvol(size_t argc, const char *argv[]);
static void shell_lsmnt(size_t argc, const char *argv[]);
static void shell_mount(size_t argc, const char *argv[]);
static void shell_unmount(size_t argc, const char *argv[]);
static void shell_lsdir(size_t argc, const char *argv[]);
static void shell_kheap_usage(size_t argc, const char *argv[]);
static void shell_uptime(size_t argc, const char *argv[]);
static void shell_run(size_t argc, const char *argv[]);

void shell_init(stream_t* out, stream_t* in, stream_t* err) {
    in_stream = in;
    out_stream = out;
    err_stream = err;

    shell_display_banner();
}

void shell_execute() {
    while(1) {
        stream_puts(out_stream, "> ");

        char *instruction = stream_gets(in_stream);

        shell_process_instruction(instruction);

        kfree(instruction);
    }
}

void shell_revert(int32_t exit_code, int32_t exception_code) {
    if(exception_code != -1) {
        stream_printf(out_stream, "Exception (%s)\n", isr_exception_messages[exception_code]);
    }

    shell_execute();
}

static void shell_display_banner() {
    const char* PATH = "A:/banner.txt";

    file_descriptor_t* banner_fd = file_open(PATH, FILE_RDONLY);

    if(!banner_fd) {
        return;
    }

    char buffer[64];
    int32_t bytes_read;

    do {
        bytes_read = file_read(banner_fd, buffer, sizeof(buffer));

        if(bytes_read > 0) {
            buffer[bytes_read] = '\0';
            stream_printf(out_stream, "%s", buffer);
        }
    } while(bytes_read > 0);

    stream_printf(out_stream, "\n\n");

    file_close(banner_fd);
}

static void shell_process_instruction(char *instruction) {
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
        shell_help(argc, argv);
    } else if(strcmp(command, "clear") == 0) {
        shell_clear(argc, argv);
    } else if(strcmp(command, "echo") == 0) {
        shell_echo(argc, argv);
    } else if(strcmp(command, "memusage") == 0) {
        shell_memory_usage(argc, argv);
    } else if(strcmp(command, "kheapusage") == 0) {
        shell_kheap_usage(argc, argv);
    } else if(strcmp(command, "memmap") == 0) {
        shell_memory_map(argc, argv);
    } else if(strcmp(command, "poweroff") == 0) {
        shell_poweroff(argc, argv);
    } else if(strcmp(command, "dmesg") == 0) {
        shell_dmesg(argc, argv);
    } else if(strcmp(command, "uptime") == 0) {
        shell_uptime(argc, argv);
    } else if(strcmp(command, "lsdev") == 0) {
        shell_lsdev(argc, argv);
    } else if(strcmp(command, "lsvol") == 0) {
        shell_lsvol(argc, argv);
    } else if(strcmp(command, "lsmnt") == 0) {
        shell_lsmnt(argc, argv);
    } else if(strcmp(command, "mount") == 0) {
        shell_mount(argc, argv);
    } else if(strcmp(command, "unmount") == 0) {
        shell_unmount(argc, argv);
    } else if(strcmp(command, "lsdir") == 0) {
        shell_lsdir(argc, argv);
    } else if(strcmp(command, "run") == 0) {
        shell_run(argc, argv);
    } else {
        stream_printf(out_stream, "Unknown command: %s\n", command);
    }

    kfree(argv);
    kfree(instruction_copy);
}

static void shell_paging(const char* buffer) {
    tty_t* tty = out_stream->data;
    char* buffer_pointer = (char*) buffer;

    while(*buffer_pointer) {
        // Check if last row has been reached
        if(tty->cursor_y == tty->rows - 1 && tty->cursor_x == 0) {
            // Display : (less) prompt
            stream_putchar(out_stream, ':');

            char ch;

            do {
                ch = stream_getchar(in_stream);

                if(ch == 'q') {
                    // Clear : (less) prompt
                    stream_putchar(out_stream, '\b');
                    return;
                }
            } while(ch != 'n');

            stream_putchar(out_stream, '\b');
        }

        // Clear : (less) prompt
        stream_putchar(out_stream, *buffer_pointer);

        buffer_pointer++;
    }

    // Display exit message
    stream_putchar(out_stream, '!');

    char ch;

    do {
        ch = stream_getchar(in_stream);
    } while(ch != 'q');

    // Clear exit message
    stream_putchar(out_stream, '\b');
}

static void shell_help(size_t argc, const char *argv[]) {
    const char* help_message = "Available commands:\n\n"
        "help - Display this help message\n"
        "clear - Clear the screen\n"
        "echo <text> - Echo the text\n"
        "memusage - Display memory usage\n"
        "kheapusage - Display kernel heap usage\n"
        "memmap - Display memory map\n"
        "poweroff - Power off the system\n"
        "dmesg - Display kernel messages\n"
        "uptime - Display system uptime\n"
        "lsdev - List available devices\n"
        "lsvol - List available volumes\n"
        "lsmnt - List available mount points\n"
        "mount <drive> <volume> - Mount a volume to a drive\n"
        "unmount <drive> - Unmount a volume from a drive\n"
        "lsdir <path> - List directory contents\n"
        "run <path> - Run a user program\n";

    shell_paging(help_message);
}

static void shell_clear(size_t argc, const char *argv[]) {
    tty_t* tty = out_stream->data;
    tty_clear(tty);
}

static void shell_echo(size_t argc, const char *argv[]) {
    if(argc < 2) {
        stream_printf(out_stream, "Usage: echo <text>\n");
        return;
    }

    stream_printf(out_stream, "%s\n", argv[1]);
}

static void shell_memory_usage(size_t argc, const char *argv[]) {
    size_t total_memory = pmm_get_total_memory_size();
    size_t free_memory = pmm_get_available_memory_size();

    double total_memory_mb = total_memory / 1024 / 1024;
    double free_memory_mb = free_memory / 1024 / 1024;
    double used_memory_mb = total_memory_mb - free_memory_mb;
    double used_memory_percentage = (used_memory_mb / total_memory_mb) * 100;

    stream_printf(out_stream, "%f MB / %f MB (%f%%) used\n", used_memory_mb, total_memory_mb, used_memory_percentage);
}

static void shell_kheap_usage(size_t argc, const char *argv[]) {
    size_t total_memory = kheap_get_total_memory_size();
    size_t free_memory = kheap_get_available_memory_size();

    double total_memory_mb = total_memory / 1024 / 1024;
    double free_memory_mb = free_memory / 1024 / 1024;
    double used_memory_mb = total_memory_mb - free_memory_mb;
    double used_memory_percentage = (used_memory_mb / total_memory_mb) * 100;

    stream_printf(out_stream, "%f MB / %f MB (%f%%) used\n", used_memory_mb, total_memory_mb, used_memory_percentage);
}

static void shell_memory_map(size_t argc, const char *argv[]) {
    const linked_list_t* pmm_memory_regions = pmm_get_memory_regions();

    linked_list_foreach(pmm_memory_regions, node) {
        pmm_memory_region_t* region = (pmm_memory_region_t*) node->data;

        stream_printf(out_stream, "Memory region: %x - %x (%d bytes) Type: %x\n", region->base, region->base + region->length - 1, region->length, region->type);
    }
}

static void shell_poweroff(size_t argc, const char *argv[]) {
    tty_t* tty = out_stream->data;

    acpi_poweroff();

    stream_printf(out_stream, "It is now safe to turn off your computer...\n");
    tty_disable_cursor(tty);

    isr_cli();
    
    while(1);
}

static void shell_dmesg(size_t argc, const char *argv[]) {
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

    shell_paging(message_buffer);

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

static void shell_lsdev(size_t argc, const char *argv[]) {
    const generic_tree_t* devices = device_get_all();

    char* message_buffer = kmalloc(1024);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    generic_tree_foreach(devices, shell_lsdev_append_callback, &message_buffer);

    shell_paging(message_buffer);

    kfree(message_buffer);
}

static void shell_lsvol(size_t argc, const char *argv[]) {
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

    shell_paging(message_buffer);

    kfree(message_buffer);
}

static void shell_lsmnt(size_t argc, const char *argv[]) {
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

    shell_paging(message_buffer);

    kfree(message_buffer);
}

static void shell_mount(size_t argc, const char *argv[]) {
    if(argc < 3) {
        stream_printf(out_stream, "Usage: mount <drive> <volume>\n");
        return;
    }

    char drive = argv[1][0];
    volume_t* volume = volume_find_by_name(argv[2]);

    if(volume == NULL) {
        stream_printf(out_stream, "Volume not found\n");
        return;
    }

    if(mnt_get_drive(drive) != NULL) {
        stream_printf(out_stream, "Drive already mounted\n");
        return;
    }

    if(mnt_volume_mount(drive, volume) != 0) {
        stream_printf(out_stream, "Failed to mount volume, file system may not be supported\n");
        return;
    }

    stream_printf(out_stream, "Volume mounted\n");
}

static void shell_unmount(size_t argc, const char *argv[]) {
    if(argc < 2) {
        stream_printf(out_stream, "Usage: unmount <drive>\n");
        return;
    }

    char drive = argv[1][0];

    if(mnt_get_drive(drive) == NULL) {
        stream_printf(out_stream, "Drive not mounted\n");
        return;
    }

    if(mnt_volume_unmount(drive) != 0) {
        stream_printf(out_stream, "Failed to unmount volume\n");
        return;
    }

    stream_printf(out_stream, "Volume unmounted\n");
}

static void shell_lsdir(size_t argc, const char *argv[]) {
    if(argc < 2) {
        stream_printf(out_stream, "Usage: lsdir <path>\n");
        return;
    }

    int32_t dd = dir_open(argv[1]);

    if(dd < 0) {
        stream_printf(out_stream, "Invalid path\n");
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

    shell_paging(message_buffer);

    kfree(message_buffer);
}

static void shell_uptime(size_t argc, const char *argv[]) {
    uint32_t uptime = timer_get_uptime();

    uint32_t weeks, days, hours, minutes, seconds;

    seconds = uptime;

    weeks = seconds / (60 * 60 * 24 * 7);
    seconds %= (60 * 60 * 24 * 7);

    days = seconds / (60 * 60 * 24);
    seconds %= (60 * 60 * 24);

    hours = seconds / (60 * 60);
    seconds %= (60 * 60);

    minutes = seconds / 60;
    seconds %= 60;

    stream_printf(out_stream, "up ");

    if(weeks > 0) {
        stream_printf(out_stream, "%d weeks, ", weeks);
    }

    if(days > 0) {
        stream_printf(out_stream, "%d days, ", days);
    }

    if(hours > 0) {
        stream_printf(out_stream, "%d hours, ", hours);
    }

    if(minutes > 0) {
        stream_printf(out_stream, "%d minutes, ", minutes);
    }

    stream_printf(out_stream, "%d seconds\n", seconds);
}

static void shell_run(size_t argc, const char *argv[]) {
    if(argc < 2) {
        stream_printf(out_stream, "Usage: run <path>\n");
        return;
    }

    process_t* process = process_create("uprogram", argv[1], out_stream, in_stream, err_stream);

    if(!process) {
        stream_puts(err_stream, "Failed to run user program\n");
        return;
    }

    process_run(process);
}
