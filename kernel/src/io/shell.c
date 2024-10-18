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
#include <uuid.h>

static void shell_process_instruction(shell_t *shell, char *instruction);
static void shell_help(shell_t *shell, char *arguments);
static void shell_clear(shell_t *shell, char *arguments);
static void shell_echo(shell_t *shell, char *arguments);
static void shell_memory_usage(shell_t *shell, char *arguments);
static void shell_memory_map(shell_t *shell, char *arguments);
static void shell_poweroff(shell_t *shell, char *arguments);
static void shell_dmesg(shell_t *shell, char *arguments);
static void shell_lsdev(shell_t *shell, char *arguments);
static void shell_lsvol(shell_t *shell, char *arguments);

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

    if(strcmp(command, "help") == 0) {
        shell_help(shell, instruction_copy);
    } else if(strcmp(command, "clear") == 0) {
        shell_clear(shell, instruction_copy); 
    } else if(strcmp(command, "echo") == 0) {
        shell_echo(shell, instruction_copy);
    } else if(strcmp(command, "memusage") == 0) {
        shell_memory_usage(shell, instruction_copy);
    } else if(strcmp(command, "memmap") == 0) {
        shell_memory_map(shell, instruction_copy);
    } else if(strcmp(command, "poweroff") == 0) {
        shell_poweroff(shell, instruction_copy);
    } else if(strcmp(command, "dmesg") == 0) {
        shell_dmesg(shell, instruction_copy);
    } else if(strcmp(command, "lsdev") == 0) {
        shell_lsdev(shell, instruction_copy);
    } else if(strcmp(command, "lsvol") == 0) {
        shell_lsvol(shell, instruction_copy);
    } else {
        tty_printf(shell->tty, "Unknown command: %s\n", command);
    }
}

static void shell_help(shell_t *shell, char *arguments) {
    const char* help_message = "Available commands:\n\n"
        "help - Display this help message\n"
        "clear - Clear the screen\n"
        "echo <text> - Echo the text\n"
        "memusage - Display memory usage\n"
        "memmap - Display memory map\n"
        "poweroff - Power off the system\n"
        "dmesg - Display kernel messages\n"
        "lsdev - List available devices\n"
        "lsvol - List available volumes\n";

    tty_paging(shell->tty, help_message);
}

static void shell_clear(shell_t *shell, char *arguments) {
    tty_clear(shell->tty);
}

static void shell_echo(shell_t *shell, char *arguments) {
    tty_printf(shell->tty, "%s\n", arguments);
}

static void shell_memory_usage(shell_t *shell, char *arguments) {
    size_t total_memory = pmm_get_total_memory_size();
    size_t free_memory = pmm_get_available_memory_size();

    double total_memory_mb = total_memory / 1024 / 1024;
    double free_memory_mb = free_memory / 1024 / 1024;
    double used_memory_mb = total_memory_mb - free_memory_mb;
    double used_memory_percentage = (used_memory_mb / total_memory_mb) * 100;

    tty_printf(shell->tty, "%f MB / %f MB (%f%%) used\n", used_memory_mb, total_memory_mb, used_memory_percentage);
}

static void shell_memory_map(shell_t *shell, char *arguments) {
    const linked_list_t* pmm_memory_regions = pmm_get_memory_regions();

    linked_list_foreach(pmm_memory_regions, node) {
        pmm_memory_region_t* region = (pmm_memory_region_t*) node->data;

        tty_printf(shell->tty, "Memory region: %x - %x (%d bytes) Type: %x\n", region->base, region->base + region->length - 1, region->length, region->type);
    }
}

static void shell_poweroff(shell_t *shell, char *arguments) {
    acpi_poweroff();

    tty_printf(shell->tty, "It is now safe to turn off your computer...\n");
    tty_disable_cursor(shell->tty);

    isr_cli();
    
    while(1);
}

static void shell_dmesg(shell_t *shell, char *arguments) {
    const linked_list_t* messages = kmessage_get_messages();

    size_t message_buffer_size = 1;

    linked_list_foreach(messages, node) {
        kmessage_message_t* message = (kmessage_message_t*) node->data;
        message_buffer_size += strlen(message->level) + strlen(message->message) + 4;
    }

    char* message_buffer = kmalloc(message_buffer_size);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    linked_list_foreach(messages, node) {
        kmessage_message_t* message = (kmessage_message_t*) node->data;

        strcat(message_buffer, "[");
        strcat(message_buffer, message->level);
        strcat(message_buffer, "] ");
        strcat(message_buffer, message->message);
        strcat(message_buffer, "\n");
    }

    tty_paging(shell->tty, message_buffer);

    kfree(message_buffer);
}

static void shell_lsdev_count_callback(generic_tree_node_t* node, void* data) {
    device_t* device = (device_t*) node->data;
    size_t* count = (size_t*) data;
    *count += strlen(device->name) + 36 + 4;
}

static void shell_lsdev_append_callback(generic_tree_node_t* node, void* data) {
    device_t* device = (device_t*) node->data;
    char* message_buffer = (char*) data;

    char uuid_buffer[37];
    uuid_v4_to_string(&device->id, uuid_buffer);

    strcat(message_buffer, device->name);
    strcat(message_buffer, " (");
    strcat(message_buffer, uuid_buffer);
    strcat(message_buffer, ")\n");
}

static void shell_lsdev(shell_t *shell, char *arguments) {
    const generic_tree_t* devices = device_get_all();

    size_t message_buffer_size = 21;

    generic_tree_foreach(devices, shell_lsdev_count_callback, &message_buffer_size);

    char* message_buffer = kmalloc(message_buffer_size);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    strcpy(message_buffer, "Available devices:\n\n");

    generic_tree_foreach(devices, shell_lsdev_append_callback, message_buffer);

    tty_paging(shell->tty, message_buffer);

    kfree(message_buffer);
}

static void shell_lsvol(shell_t *shell, char *arguments) {
    const linked_list_t* volumes = volume_get_all();

    size_t message_buffer_size = 21;

    linked_list_foreach(volumes, node) {
        volume_t* volume = (volume_t*) node->data;
        message_buffer_size += strlen(volume->name) + 36 + 4;
    }

    char* message_buffer = kmalloc(message_buffer_size);

    if(message_buffer == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    message_buffer[0] = '\0';

    strcpy(message_buffer, "Available volumes:\n\n");

    linked_list_foreach(volumes, node) {
        volume_t* volume = (volume_t*) node->data;

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
