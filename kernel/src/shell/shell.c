#include <shell/shell.h>
#include <util/string.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <system/process.h>
#include <arch/i386/isr.h>
#include <io/file.h>
#include <io/dir.h>
#include <io/tty.h>

static stream_t* in_stream = NULL;
static stream_t* out_stream = NULL;
static stream_t* err_stream = NULL;

static void shell_display_banner();
static void shell_process_instruction(char *instruction);
static void shell_paging(const char* buffer);
static void shell_help(size_t argc, const char *argv[]);
static void shell_clear(size_t argc, const char *argv[]);
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
        "run <path> - Run a user program\n";

    shell_paging(help_message);
}

static void shell_clear(size_t argc, const char *argv[]) {
    tty_t* tty = out_stream->data;
    tty_clear(tty);
}

static void shell_run(size_t argc, const char *argv[]) {
    if(argc < 2) {
        stream_printf(out_stream, "Usage: run <path>\n");
        return;
    }

    process_t* process = process_create("uprogram", argv[1], argc - 1, &argv[1], out_stream, in_stream, err_stream);

    if(!process) {
        stream_puts(err_stream, "Failed to run user program\n");
        return;
    }

    process_run(process);
}
