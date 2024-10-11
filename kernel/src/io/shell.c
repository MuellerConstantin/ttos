#include <io/shell.h>
#include <string.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>
#include <memory/pmm.h>

static void shell_process_instruction(shell_t *shell, char *instruction);
static void shell_echo(shell_t *shell, char *arguments);
static void shell_memory_usage(shell_t *shell, char *arguments);
static void shell_memory_map(shell_t *shell, char *arguments);

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

    if(strcmp(command, "echo") == 0) {
        shell_echo(shell, instruction_copy);
    } else if(strcmp(command, "memusage") == 0) {
        shell_memory_usage(shell, instruction_copy);
    } else if(strcmp(command, "memmap") == 0) {
        shell_memory_map(shell, instruction_copy);
    } else {
        tty_printf(shell->tty, "Unknown command: %s\n", command);
    }
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
