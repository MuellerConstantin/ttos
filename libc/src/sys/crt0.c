#include <stddef.h>

extern int main(int argc, char **argv,char **envp);

void _start() {
    main(0, NULL, NULL);

    while (1);
}
