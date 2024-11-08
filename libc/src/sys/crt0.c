#include <stddef.h>
#include <proc.h>

extern int main(int argc, char **argv, char **envp);

void _start() {
    _exit(main(0, NULL, NULL));
}
