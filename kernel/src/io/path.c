#include <io/path.h>
#include <util/string.h>

static bool path_is_drive_letter(char c);
static int32_t path_append(char* resolved, const char* components);

int32_t path_resolve(const char* cwd, const char* path, char* resolved) {
    if(!cwd || !path || !resolved) {
        return -1;
    }

    const char* components;

    if(path_is_drive_letter(path[0]) && path[1] == ':') {
        if(path[2] != '/') {
            return -1;
        }

        resolved[0] = path[0];
        components = path + 2;
    } else {
        resolved[0] = cwd[0];
        components = path;
    }

    // Normalize the drive letter so that paths compare equal regardless of how they were typed.
    if(resolved[0] >= 'a' && resolved[0] <= 'z') {
        resolved[0] -= 'a' - 'A';
    }

    resolved[1] = ':';
    resolved[2] = '\0';

    // A relative path continues from the working directory, so its components come first.
    if(components[0] != '/') {
        if(path_append(resolved, cwd + 2) != 0) {
            return -1;
        }
    }

    if(path_append(resolved, components) != 0) {
        return -1;
    }

    // The root of a drive keeps its slash, it is what makes the path absolute.
    if(resolved[2] == '\0') {
        resolved[2] = '/';
        resolved[3] = '\0';
    }

    return 0;
}

static bool path_is_drive_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/*
 * Appends the components of a slash separated path to a resolved path of the
 * form X: or X:/a/b, folding '.' and '..' as it goes. '..' at the root stays at
 * the root. Fails if the result would not fit into PATH_MAX bytes.
 */
static int32_t path_append(char* resolved, const char* components) {
    size_t length = strlen(resolved);

    while(*components) {
        // Skip separators, including repeated ones.
        if(*components == '/') {
            components++;
            continue;
        }

        size_t component_length = 0;

        while(components[component_length] && components[component_length] != '/') {
            component_length++;
        }

        if(component_length == 1 && components[0] == '.') {
            components += component_length;
            continue;
        }

        if(component_length == 2 && components[0] == '.' && components[1] == '.') {
            // Drop the last component; the drive prefix X: itself is never removed.
            while(length > 2 && resolved[length - 1] != '/') {
                length--;
            }

            if(length > 2) {
                length--;
            }

            resolved[length] = '\0';
            components += component_length;
            continue;
        }

        if(length + 1 + component_length >= PATH_MAX) {
            return -1;
        }

        resolved[length++] = '/';
        memcpy(resolved + length, components, component_length);
        length += component_length;
        resolved[length] = '\0';

        components += component_length;
    }

    return 0;
}
