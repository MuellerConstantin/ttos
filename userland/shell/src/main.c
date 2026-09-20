#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fsio.h>
#include <proc.h>
#include <ttos/syscall.h>

#define SHELL_LINE_MAX 256
#define SHELL_MAX_ARGS 32
#define SHELL_PATH_MAX 128
#define SHELL_HISTORY_MAX 16

/*
 * The command search path lives in the PATH environment variable so that it
 * reaches the shell from init and any program the shell starts. Entries are
 * separated by ';' - ':' is taken by the drive letters. Where no PATH is
 * inherited the initrd root is searched.
 */
#define SHELL_PATH_VARIABLE "PATH"
#define SHELL_PATH_SEPARATOR ';'
#define SHELL_PATH_DEFAULT "A:/"

/** Room for the whole value of an environment variable when it is rebuilt. */
#define SHELL_ENV_MAX 512

/*
 * The shell paints the terminal in its own background once it takes over, so
 * anything that resets attributes has to restore that background rather than
 * the terminal default, which is black.
 */
#define SHELL_BACKGROUND "\033[44m"
#define SHELL_RESET "\033[0m" SHELL_BACKGROUND

/*
 * The prompt is the working directory followed by this suffix. The directory
 * is drawn in its own color so that it stands apart from the suffix and the
 * input; the suffix takes the terminal's default foreground. Text and style
 * are kept apart on purpose: the redraw counts strlen(prompt) as the prompt's
 * width on screen, so escape sequences must never be part of the string
 * itself.
 */
#define SHELL_PROMPT_SUFFIX "> "
#define SHELL_PROMPT_PATH_STYLE "\033[93m"

/** Room for the working directory plus the prompt suffix. */
#define SHELL_PROMPT_MAX (PATH_MAX + sizeof(SHELL_PROMPT_SUFFIX))

// Ring of recently entered command lines, navigated with the up/down arrows.
static char history[SHELL_HISTORY_MAX][SHELL_LINE_MAX];
static size_t history_count = 0;

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
    printf("set [NAME=VALUE] - Show the environment or set a variable\n");
    printf("cd [<dir>] - Change the working directory, or show it\n");
    printf("pwd - Show the working directory\n");
    printf("<command> [args...] - Run a program, resolved via the search path\n");
    printf("<path> [args...] - Run a program by its full path\n");
    printf("<command> [args...] & - Run a program in the background\n");
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

/*
 * Copies the next entry of a ';' separated list into `entry`, without a
 * trailing slash. Returns where the following entry starts, or NULL once the
 * list is exhausted. Empty entries are skipped.
 */
static const char* shell_path_next(const char* cursor, char* entry) {
    while(cursor && *cursor) {
        size_t length = 0;

        while(cursor[length] && cursor[length] != SHELL_PATH_SEPARATOR) {
            length++;
        }

        const char* next = cursor[length] ? cursor + length + 1 : cursor + length;

        if(length > 0 && length < SHELL_PATH_MAX) {
            strncpy(entry, cursor, length);
            entry[length] = '\0';

            if(length > 1 && entry[length - 1] == '/') {
                entry[length - 1] = '\0';
            }

            return next;
        }

        cursor = next;
    }

    return NULL;
}

/*
 * Strips a trailing slash so that `C:/bin` and `C:/bin/` name the same entry.
 * Returns 0 if the directory is unusable as a search path entry.
 */
static int shell_path_normalize(const char* directory, char* entry) {
    size_t length = strlen(directory);

    if(length == 0 || length >= SHELL_PATH_MAX) {
        return 0;
    }

    strcpy(entry, directory);

    if(length > 1 && entry[length - 1] == '/') {
        entry[length - 1] = '\0';
    }

    return 1;
}

static void shell_path(size_t argc, char** argv) {
    const char* path = getenv(SHELL_PATH_VARIABLE);
    char entry[SHELL_PATH_MAX];

    if(argc == 1) {
        for(const char* cursor = shell_path_next(path, entry); cursor; cursor = shell_path_next(cursor, entry)) {
            printf("%s\n", entry);
        }

        return;
    }

    if(argc >= 3 && strcmp(argv[1], "add") == 0) {
        char addition[SHELL_PATH_MAX];

        if(!shell_path_normalize(argv[2], addition)) {
            printf("path: invalid directory\n");
            return;
        }

        size_t current_length = path ? strlen(path) : 0;

        if(current_length + 1 + strlen(addition) >= SHELL_ENV_MAX) {
            printf("path: search path is full\n");
            return;
        }

        char value[SHELL_ENV_MAX];
        value[0] = '\0';

        if(current_length > 0) {
            strcpy(value, path);
            value[current_length] = SHELL_PATH_SEPARATOR;
            value[current_length + 1] = '\0';
        }

        strcat(value, addition);

        if(setenv(SHELL_PATH_VARIABLE, value, 1) != 0) {
            printf("path: out of memory\n");
        }

        return;
    }

    if(argc >= 3 && strcmp(argv[1], "remove") == 0) {
        char query[SHELL_PATH_MAX];

        if(!shell_path_normalize(argv[2], query)) {
            printf("path: invalid directory\n");
            return;
        }

        // Rebuild the value from every entry but the one being removed.
        char value[SHELL_ENV_MAX];
        value[0] = '\0';
        int found = 0;

        for(const char* cursor = shell_path_next(path, entry); cursor; cursor = shell_path_next(cursor, entry)) {
            if(strcmp(entry, query) == 0) {
                found = 1;
                continue;
            }

            if(value[0] != '\0') {
                size_t length = strlen(value);
                value[length] = SHELL_PATH_SEPARATOR;
                value[length + 1] = '\0';
            }

            strcat(value, entry);
        }

        if(!found) {
            printf("path: not in search path: %s\n", argv[2]);
            return;
        }

        if(setenv(SHELL_PATH_VARIABLE, value, 1) != 0) {
            printf("path: out of memory\n");
        }

        return;
    }

    printf("usage: path [add <dir> | remove <dir>]\n");
}

static void shell_set(size_t argc, char** argv) {
    if(argc == 1) {
        for(char** entry = environ; entry && *entry; entry++) {
            printf("%s\n", *entry);
        }

        return;
    }

    char* separator = strpbrk(argv[1], "=");

    if(argc != 2 || !separator || separator == argv[1]) {
        printf("usage: set [NAME=VALUE]\n");
        return;
    }

    // Split the argument in place; it is the shell's own line buffer.
    *separator = '\0';

    if(setenv(argv[1], separator + 1, 1) != 0) {
        printf("set: out of memory\n");
    }
}

static void shell_pwd(void) {
    char cwd[PATH_MAX];

    if(getcwd(cwd, sizeof(cwd)) != 0) {
        printf("pwd: cannot read working directory\n");
        return;
    }

    printf("%s\n", cwd);
}

static void shell_cd(size_t argc, char** argv) {
    if(argc == 1) {
        shell_pwd();
        return;
    }

    if(argc != 2) {
        printf("usage: cd [<dir>]\n");
        return;
    }

    if(chdir(argv[1]) != 0) {
        printf("cd: no such directory: %s\n", argv[1]);
    }
}

/*
 * Builds the prompt from the current working directory. The directory can
 * change between two lines, so this runs before every read.
 */
static void shell_prompt(char* prompt) {
    if(getcwd(prompt, PATH_MAX) != 0) {
        prompt[0] = '\0';
    }

    strcat(prompt, SHELL_PROMPT_SUFFIX);
}

/*
 * Prints the prompt with the directory in its own color. The prompt always
 * ends in the suffix, so the directory is everything before it.
 */
static void shell_print_prompt(const char* prompt) {
    size_t path_length = strlen(prompt) - (sizeof(SHELL_PROMPT_SUFFIX) - 1);

    printf(SHELL_RESET SHELL_PROMPT_PATH_STYLE);

    for(size_t index = 0; index < path_length; index++) {
        putchar(prompt[index]);
    }

    printf(SHELL_RESET "%s", prompt + path_length);
}

/*
 * Tries to start `path`, then `path.elf` if it does not already end in .elf.
 * Returns the PID of the started program, or -1 if neither could be started.
 */
static pid_t shell_try_spawn(const char* path, char** argv) {
    pid_t result = spawn(path, argv);

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
 * Resolves argv[0] to an executable and starts it. A name that contains a
 * drive or a path separator is used as given, relative names being resolved
 * by the kernel against the working directory; a bare name is looked up in the
 * search path. Returns the PID of the started program, or -1 if nothing could
 * be executed.
 */
static pid_t shell_run(char** argv) {
    if(strpbrk(argv[0], ":/") != NULL) {
        return shell_try_spawn(argv[0], argv);
    }

    char entry[SHELL_PATH_MAX];

    for(const char* cursor = shell_path_next(getenv(SHELL_PATH_VARIABLE), entry); cursor; cursor = shell_path_next(cursor, entry)) {
        char candidate[SHELL_PATH_MAX];

        if(strlen(entry) + 1 + strlen(argv[0]) >= SHELL_PATH_MAX) {
            continue;
        }

        strcpy(candidate, entry);
        strcat(candidate, "/");
        strcat(candidate, argv[0]);

        pid_t result = shell_try_spawn(candidate, argv);

        if(result >= 0) {
            return result;
        }
    }

    return -1;
}

/*
 * Collects background jobs that have finished and reports them. Foreground
 * commands are waited for directly, so whatever wait finds here ran in the
 * background.
 */
static void shell_reap(void) {
    int status;
    pid_t pid;

    while((pid = wait(-1, &status, WAIT_NOHANG)) > 0) {
        printf("[%d] exited (%d)\n", pid, status);
    }
}

static void shell_history_add(const char* line) {
    // Ignore empty lines and consecutive duplicates.
    if(line[0] == '\0') {
        return;
    }

    if(history_count > 0 && strcmp(history[history_count - 1], line) == 0) {
        return;
    }

    // Drop the oldest entry once the ring is full.
    if(history_count == SHELL_HISTORY_MAX) {
        for(size_t index = 0; index + 1 < SHELL_HISTORY_MAX; index++) {
            strcpy(history[index], history[index + 1]);
        }

        history_count--;
    }

    strcpy(history[history_count], line);
    history_count++;
}

/*
 * Redraws the whole input line and leaves the terminal cursor at `cursor`.
 * `drawn` is the logical cursor position the terminal still shows, i.e. where
 * it was before the caller edited the buffer.
 *
 * All movement is relative and counted in characters, so this holds up once the
 * line wraps onto further terminal rows: \033[C and \033[D carry over into the
 * neighbouring row, and \033[0J clears the rows a now shorter line vacated.
 */
static void shell_redraw(const char* prompt, const char* buffer, size_t length, size_t cursor, size_t drawn) {
    size_t offset = strlen(prompt) + drawn;

    // Walk back over prompt and text to the start of the line.
    if(offset > 0) {
        printf("\033[%dD", (int) offset);
    }

    /*
     * Reset attributes first: a program may have exited mid-color, and \033[0J
     * would otherwise smear that background across the rest of the screen.
     */
    shell_print_prompt(prompt);
    printf("%s\033[0J", buffer);

    /*
     * Reprinting left the terminal cursor at the end of the buffer; move it back
     * to the logical cursor position if that sits before the end.
     */
    if(cursor < length) {
        printf("\033[%dD", (int) (length - cursor));
    }
}

/*
 * Reads a line of input with echo, in-line editing and history recall. Handles
 * Enter, Backspace, up/down (history) and left/right (cursor movement, with
 * mid-line insertion and deletion). The line may wrap onto further rows.
 */
static void shell_read_line(const char* prompt, char* buffer, size_t size) {
    size_t length = 0;
    size_t cursor = 0;
    size_t nav = history_count;

    buffer[0] = '\0';
    shell_print_prompt(prompt);

    for(;;) {
        int ch = getchar();

        if(ch == '\n') {
            /*
             * Step past any tail sitting to the right of the cursor so the
             * newline starts below the whole line, not in the middle of it.
             */
            if(cursor < length) {
                printf("\033[%dC", (int) (length - cursor));
            }

            putchar('\n');
            break;
        }

        if(ch == '\b') {
            if(cursor > 0) {
                size_t drawn = cursor;

                // Delete the character before the cursor, shifting the tail left.
                for(size_t i = cursor - 1; i + 1 < length; i++) {
                    buffer[i] = buffer[i + 1];
                }

                length--;
                cursor--;
                buffer[length] = '\0';
                shell_redraw(prompt, buffer, length, cursor, drawn);
            }

            continue;
        }

        if(ch == 0x1B) {
            // ANSI escape sequence: ESC '[' <final byte>.
            if(getchar() != '[') {
                continue;
            }

            int final = getchar();

            if(final == 'A' || final == 'B') {
                // History navigation.
                size_t drawn = cursor;

                if(final == 'A') {
                    if(nav > 0) {
                        nav--;
                    } else {
                        continue;
                    }
                } else {
                    if(nav < history_count) {
                        nav++;
                    } else {
                        continue;
                    }
                }

                if(nav < history_count) {
                    strcpy(buffer, history[nav]);
                } else {
                    buffer[0] = '\0';
                }

                length = strlen(buffer);
                cursor = length;
                shell_redraw(prompt, buffer, length, cursor, drawn);
            } else if(final == 'C') {
                // Cursor right.
                if(cursor < length) {
                    cursor++;
                    printf("\033[C");
                }
            } else if(final == 'D') {
                // Cursor left.
                if(cursor > 0) {
                    cursor--;
                    printf("\033[D");
                }
            }

            continue;
        }

        // Printable input: insert at the cursor. Leave room for the terminator.
        if(length + 1 < size) {
            if(cursor == length) {
                // Fast path: appending at the end of the line.
                buffer[length] = (char) ch;
                length++;
                cursor++;
                buffer[length] = '\0';
                putchar(ch);
            } else {
                // Insert in the middle: shift the tail right, then redraw.
                size_t drawn = cursor;

                for(size_t i = length; i > cursor; i--) {
                    buffer[i] = buffer[i - 1];
                }

                buffer[cursor] = (char) ch;
                length++;
                cursor++;
                buffer[length] = '\0';
                shell_redraw(prompt, buffer, length, cursor, drawn);
            }
        }
    }

    buffer[length] = '\0';

    shell_history_add(buffer);
}

int main(void) {
    /*
     * Take over the terminal: set the background, then clear so it covers the
     * whole screen rather than only the cells written from here on.
     */
    printf(SHELL_BACKGROUND "\033[2J\033[H");

    shell_banner();

    setenv(SHELL_PATH_VARIABLE, SHELL_PATH_DEFAULT, 0);

    char line[SHELL_LINE_MAX];
    char prompt[SHELL_PROMPT_MAX];
    char* argv[SHELL_MAX_ARGS + 1];

    for(;;) {
        shell_reap();

        shell_prompt(prompt);
        shell_read_line(prompt, line, sizeof(line));

        size_t argc = shell_tokenize(line, argv, SHELL_MAX_ARGS);

        if(argc == 0) {
            continue;
        }

        // A trailing & runs the command in the background.
        int background = strcmp(argv[argc - 1], "&") == 0;

        if(background && --argc == 0) {
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

        if(strcmp(argv[0], "set") == 0) {
            shell_set(argc, argv);
            continue;
        }

        if(strcmp(argv[0], "cd") == 0) {
            shell_cd(argc, argv);
            continue;
        }

        if(strcmp(argv[0], "pwd") == 0) {
            shell_pwd();
            continue;
        }

        pid_t pid = shell_run(argv);

        if(pid < 0) {
            printf("shell: command not found: %s\n", argv[0]);
            continue;
        }

        if(background) {
            printf("[%d]\n", pid);
        } else {
            wait(pid, NULL, 0);
        }
    }

    return 0;
}
