#include <stdlib.h>
#include <string.h>
#include <proc.h>

char* getenv(const char* name) {
    if(!name || !environ) {
        return NULL;
    }

    size_t length = strlen(name);

    for(char** entry = environ; *entry; entry++) {
        if(strncmp(*entry, name, length) == 0 && (*entry)[length] == '=') {
            return *entry + length + 1;
        }
    }

    return NULL;
}
