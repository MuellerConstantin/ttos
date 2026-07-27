#include <stdio.h>
#include <string.h>
#include <fsio.h>
#include <proc.h>

#define SHELL_LINE_MAX 256
#define SHELL_MAX_ARGS 32

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
    printf("<path> [args...] - Run a program, e.g. A:/bin/lsvol.elf\n");
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

int main(void) {
    shell_banner();

    char line[SHELL_LINE_MAX];
    char* argv[SHELL_MAX_ARGS + 1];

    for(;;) {
        printf("> ");

        gets(line);

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

        int code = spawn(argv[0], argv);

        if(code < 0) {
            printf("shell: cannot run %s\n", argv[0]);
        }
    }

    return 0;
}
