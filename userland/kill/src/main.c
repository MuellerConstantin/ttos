#include <proc.h>
#include <stdio.h>

/* Parses a PID: decimal digits only. Returns -1 for anything else. */
static pid_t parse_pid(const char* text) {
    pid_t pid = 0;

    if (*text == '\0') {
        return -1;
    }

    for (; *text != '\0'; text++) {
        if (*text < '0' || *text > '9') {
            return -1;
        }

        pid = pid * 10 + (*text - '0');
    }

    return pid;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("usage: kill <pid>\n");
        return 1;
    }

    pid_t pid = parse_pid(argv[1]);

    if (pid < 0) {
        printf("kill: not a pid: %s\n", argv[1]);
        return 1;
    }

    if (kill(pid) != 0) {
        printf("kill: no such process: %d\n", pid);
        return 1;
    }

    return 0;
}
