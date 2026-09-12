#include <stdlib.h>
#include <string.h>
#include <proc.h>

/*
 * The pointer array setenv last allocated. As long as environ still points at
 * it the array can be grown in place; anything else - the array the kernel
 * placed on the initial stack, or one the program installed itself - is left
 * alone and copied before it is extended.
 */
static char** owned = NULL;

int setenv(const char* name, const char* value, int overwrite) {
    if(!name || !value || name[0] == '\0' || strpbrk(name, "=")) {
        return -1;
    }

    size_t name_length = strlen(name);
    size_t count = 0;
    char** existing = NULL;

    if(environ) {
        for(char** entry = environ; *entry; entry++, count++) {
            if(!existing && strncmp(*entry, name, name_length) == 0 && (*entry)[name_length] == '=') {
                existing = entry;
            }
        }
    }

    if(existing && !overwrite) {
        return 0;
    }

    char* assignment = malloc(name_length + 1 + strlen(value) + 1);

    if(!assignment) {
        return -1;
    }

    strcpy(assignment, name);
    strcat(assignment, "=");
    strcat(assignment, value);

    if(existing) {
        *existing = assignment;
        return 0;
    }

    // One more entry plus the NULL terminator.
    char** grown;

    if(owned && environ == owned) {
        grown = realloc(owned, (count + 2) * sizeof(char*));
    } else {
        grown = malloc((count + 2) * sizeof(char*));

        if(grown) {
            for(size_t index = 0; index < count; index++) {
                grown[index] = environ[index];
            }
        }
    }

    if(!grown) {
        free(assignment);
        return -1;
    }

    grown[count] = assignment;
    grown[count + 1] = NULL;

    owned = grown;
    environ = grown;

    return 0;
}
