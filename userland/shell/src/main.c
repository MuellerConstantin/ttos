#include <stdio.h>
#include <string.h>
#include <fsio.h>
#include <proc.h>

#define SHELL_LINE_MAX 256
#define SHELL_MAX_ARGS 32
#define SHELL_PATH_MAX 128
#define SHELL_MAX_PATHS 8

// Directories searched for bare command names, in order. Starts with the initrd
// root and can be extended at runtime with the `path` builtin.
static char search_paths[SHELL_MAX_PATHS][SHELL_PATH_MAX];
static size_t search_path_count = 0;

static void shell_banner(void) {
    int32_t fd = fsio_open("A:/banner.txt", FSIO_RDONLY, 0);

    if(fd < 0) {
        return;
    }

    char buffer[64];
    int32_t bytes_read;

    while((bytes_read = fsio_read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    fsio_close(fd);

    printf("\n\n");
}

static void shell_help(void) {
    printf("Available commands:\n\n");
    printf("help - Display this help message\n");
    printf("path [add <dir> | remove <dir>] - Show or edit the command search path\n");
    printf("<command> [args...] - Run a program, resolved via the search path\n");
    printf("<path> [args...] - Run a program by its full path\n");
}

static int shell_ends_with(const char* string, const char* suffix) {
    size_t string_length = strlen(string);
    size_t suffix_length = strlen(suffix);

    if(suffix_length > string_length) {
        return 0;
    }

    return strcmp(string + (string_length - suffix_length), suffix) == 0;
}

/*
 * Splits a line into whitespace-separated tokens, honoring simple double-quoted
 * arguments. Modifies the line in place and stores pointers into it in argv.
 */
static size_t shell_tokenize(char* line, char** argv, size_t max_args) {
    size_t argc = 0;
    char* rest = line;

    while(rest != NULL && argc < max_args) {
        // Skip leading spaces
        while(*rest == ' ') {
            rest++;
        }

        if(*rest == '\0') {
            break;
        }

        char* token;

        if(*rest == '"') {
            rest++;
            token = strsep(&rest, "\"");
        } else {
            token = strsep(&rest, " ");
        }

        if(token != NULL && *token != '\0') {
            argv[argc++] = token;
        }
    }

    return argc;
}

static void shell_path(size_t argc, char** argv) {
    if(argc == 1) {
        for(size_t index = 0; index < search_path_count; index++) {
            printf("%s\n", search_paths[index]);
        }

        return;
    }

    if(argc >= 3 && strcmp(argv[1], "add") == 0) {
        size_t length = strlen(argv[2]);

        if(length == 0 || length >= SHELL_PATH_MAX - 1) {
            printf("path: invalid directory\n");
            return;
        }

        if(search_path_count >= SHELL_MAX_PATHS) {
            printf("path: search path is full\n");
            return;
        }

        char* entry = search_paths[search_path_count];
        strcpy(entry, argv[2]);

        // Normalize to a trailing slash so candidates concatenate cleanly.
        if(entry[length - 1] != '/') {
            entry[length] = '/';
            entry[length + 1] = '\0';
        }

        search_path_count++;
        return;
    }

    if(argc >= 3 && strcmp(argv[1], "remove") == 0) {
        // Normalize the query the same way entries are stored before comparing.
        char query[SHELL_PATH_MAX];
        size_t length = strlen(argv[2]);

        if(length == 0 || length >= SHELL_PATH_MAX - 1) {
            printf("path: invalid directory\n");
            return;
        }

        strcpy(query, argv[2]);

        if(query[length - 1] != '/') {
            query[length] = '/';
            query[length + 1] = '\0';
        }

        for(size_t index = 0; index < search_path_count; index++) {
            if(strcmp(search_paths[index], query) == 0) {
                for(size_t shift = index; shift + 1 < search_path_count; shift++) {
                    strcpy(search_paths[shift], search_paths[shift + 1]);
                }

                search_path_count--;
                return;
            }
        }

        printf("path: not in search path: %s\n", argv[2]);
        return;
    }

    printf("usage: path [add <dir> | remove <dir>]\n");
}

/*
 * Tries to run `path`, then `path.elf` if it does not already end in .elf.
 * Returns the program's status if it ran, or -1 if neither could be started.
 */
static int shell_try_spawn(const char* path, char** argv) {
    int result = spawn(path, argv);

    if(result >= 0) {
        return result;
    }

    if(!shell_ends_with(path, ".elf")) {
        char with_extension[SHELL_PATH_MAX];

        if(strlen(path) + 4 < SHELL_PATH_MAX) {
            strcpy(with_extension, path);
            strcat(with_extension, ".elf");

            result = spawn(with_extension, argv);

            if(result >= 0) {
                return result;
            }
        }
    }

    return -1;
}

/*
 * Resolves argv[0] to an executable and runs it. Drive-qualified names (those
 * containing ':') are used as given; bare names are looked up in the search
 * path. Returns the program's status, or -1 if nothing could be executed.
 */
static int shell_run(char** argv) {
    if(strpbrk(argv[0], ":") != NULL) {
        return shell_try_spawn(argv[0], argv);
    }

    for(size_t index = 0; index < search_path_count; index++) {
        char candidate[SHELL_PATH_MAX];

        if(strlen(search_paths[index]) + strlen(argv[0]) >= SHELL_PATH_MAX) {
            continue;
        }

        strcpy(candidate, search_paths[index]);
        strcat(candidate, argv[0]);

        int result = shell_try_spawn(candidate, argv);

        if(result >= 0) {
            return result;
        }
    }

    return -1;
}

/*
 * Reads a line of input character by character, echoing as it goes. Handles
 * Enter (terminates the line) and Backspace (erases the previous character).
 * Input that would overflow the buffer is dropped and not echoed, so the
 * display stays in sync with the buffer. This is the shell's own line editor,
 * replacing libc gets(), and is where escape-sequence handling (arrow keys,
 * history) will later live.
 */
static void shell_read_line(char* buffer, size_t size) {
    size_t index = 0;

    for(;;) {
        int ch = getchar();

        if(ch == '\n') {
            putchar('\n');
            break;
        }

        if(ch == '\b') {
            if(index > 0) {
                index--;
                putchar('\b');
            }

            continue;
        }

        // Leave room for the null terminator.
        if(index + 1 < size) {
            buffer[index++] = (char) ch;
            putchar(ch);
        }
    }

    buffer[index] = '\0';
}

int main(void) {
    shell_banner();

    strcpy(search_paths[0], "A:/");
    search_path_count = 1;

    char line[SHELL_LINE_MAX];
    char* argv[SHELL_MAX_ARGS + 1];

    for(;;) {
        printf("> ");

        shell_read_line(line, sizeof(line));

        size_t argc = shell_tokenize(line, argv, SHELL_MAX_ARGS);

        if(argc == 0) {
            continue;
        }

        // Terminate the argument vector for spawn.
        argv[argc] = 0;

        if(strcmp(argv[0], "help") == 0) {
            shell_help();
            continue;
        }

        if(strcmp(argv[0], "path") == 0) {
            shell_path(argc, argv);
            continue;
        }

        if(shell_run(argv) < 0) {
            printf("shell: command not found: %s\n", argv[0]);
        }
    }

    return 0;
}
