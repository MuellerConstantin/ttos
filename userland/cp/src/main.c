#include <fsio.h>
#include <dirio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ttos/syscall.h>

/*
 * Copied in chunks of this size. The buffer is static: the initial user stack
 * is a single page and shared with the arguments and the environment.
 */
#define CP_BUFFER_SIZE 4096

/*
 * Deepest directory nesting a recursive copy follows. Each level holds its
 * paths and directory entry on the heap, so the stack only carries the call
 * frames themselves, but those still add up on a single page.
 */
#define CP_MAX_DEPTH 32

static char buffer[CP_BUFFER_SIZE];

/*
 * Identity of the directory a recursive copy writes into. A directory met on
 * the way down that turns out to be this one is skipped, otherwise the copy
 * would descend into its own output without end.
 */
static fileinfo_t target_root;
static int target_root_known = 0;

/** The paths and the directory entry one level of the recursion works with. */
typedef struct {
    char source[PATH_MAX];
    char target[PATH_MAX];
    dirent_t entry;
} cp_level_t;

static const char* cp_basename(const char* path);
static int cp_join(const char* directory, const char* name, char* out);
static int cp_same_file(const fileinfo_t* a, const fileinfo_t* b);
static int cp_file(const char* source, const fileinfo_t* source_info, const char* target);
static int cp_tree(const char* source, const char* target, int depth);

int main(int argc, char** argv) {
    int recursive = 0;

    if (argc >= 2 && strcmp(argv[1], "-r") == 0) {
        recursive = 1;
        argv++;
        argc--;
    }

    if (argc != 3) {
        puts("usage: cp [-r] <source> <destination>\n");
        return 1;
    }

    const char* source = argv[1];
    const char* destination = argv[2];
    char target[PATH_MAX];
    fileinfo_t source_info;
    fileinfo_t destination_info;

    if (fsio_stat(source, &source_info) != 0) {
        printf("cp: cannot access %s\n", source);
        return 1;
    }

    if (source_info.type == FILE_TYPE_DIRECTORY && !recursive) {
        printf("cp: %s is a directory, use -r\n", source);
        return 1;
    }

    /*
     * A destination that is a directory receives the source under its own
     * name; anything else is the name of the copy itself.
     */
    int destination_is_directory = fsio_stat(destination, &destination_info) == 0 && destination_info.type == FILE_TYPE_DIRECTORY;

    if (destination_is_directory) {
        if (cp_join(destination, cp_basename(source), target) != 0) {
            puts("cp: path too long\n");
            return 1;
        }
    } else {
        if (strlen(destination) >= PATH_MAX) {
            puts("cp: path too long\n");
            return 1;
        }

        strcpy(target, destination);
    }

    if (source_info.type == FILE_TYPE_DIRECTORY) {
        return cp_tree(source, target, 0);
    }

    return cp_file(source, &source_info, target);
}

/** The last component of a path, or the path itself if it has none. */
static const char* cp_basename(const char* path) {
    const char* base = path;

    for (const char* cursor = path; *cursor; cursor++) {
        if (*cursor == '/' && cursor[1] != '\0') {
            base = cursor + 1;
        }
    }

    return base;
}

/** Writes directory/name into out. Fails if the result does not fit. */
static int cp_join(const char* directory, const char* name, char* out) {
    size_t length = strlen(directory);

    if (length + 1 + strlen(name) >= PATH_MAX) {
        return -1;
    }

    strcpy(out, directory);

    if (length > 0 && out[length - 1] != '/') {
        out[length++] = '/';
        out[length] = '\0';
    }

    strcat(out, name);

    return 0;
}

/*
 * Whether two paths name the same object. Volume and inode identify a file;
 * the type is compared as well, since the initial ramdisk numbers its root
 * and its first file alike.
 */
static int cp_same_file(const fileinfo_t* a, const fileinfo_t* b) {
    return a->type == b->type && a->inode == b->inode && strcmp(a->volume_id, b->volume_id) == 0;
}

/** Copies one file. Returns 0 on success or 1 after reporting the failure. */
static int cp_file(const char* source, const fileinfo_t* source_info, const char* target) {
    fileinfo_t target_info;

    /*
     * Opening the target truncates it, so a target that is the source would be
     * emptied before a byte is read. An existing target is compared by identity,
     * which catches the same file under any two names.
     */
    if (fsio_stat(target, &target_info) == 0 && cp_same_file(source_info, &target_info)) {
        printf("cp: %s and %s are the same file\n", source, target);
        return 1;
    }

    int32_t in = fsio_open(source, FSIO_RDONLY, 0);

    if (in < 0) {
        printf("cp: cannot open %s\n", source);
        return 1;
    }

    int32_t out = fsio_open(target, FSIO_WRONLY | FSIO_CREAT | FSIO_TRUNC, source_info->permissions);

    if (out < 0) {
        printf("cp: cannot create %s\n", target);
        fsio_close(in);
        return 1;
    }

    int status = 0;
    int32_t bytes_read;

    while ((bytes_read = fsio_read(in, buffer, sizeof(buffer))) > 0) {
        // A short write is what a full file system looks like from here.
        if (fsio_write(out, buffer, bytes_read) != bytes_read) {
            printf("cp: write failed: %s\n", target);
            status = 1;
            break;
        }
    }

    if (bytes_read < 0) {
        printf("cp: read failed: %s\n", source);
        status = 1;
    }

    fsio_close(out);
    fsio_close(in);

    return status;
}

/*
 * Copies the directory source into target, creating target if it does not
 * exist. Failures are reported and the copy goes on with the next entry;
 * the return value tells whether everything succeeded.
 */
static int cp_tree(const char* source, const char* target, int depth) {
    if (depth >= CP_MAX_DEPTH) {
        printf("cp: %s is nested too deep\n", source);
        return 1;
    }

    fileinfo_t target_info;

    if (fsio_stat(target, &target_info) == 0) {
        if (target_info.type != FILE_TYPE_DIRECTORY) {
            printf("cp: %s is not a directory\n", target);
            return 1;
        }
    } else if (fsio_mkdir(target, 0755) != 0 || fsio_stat(target, &target_info) != 0) {
        printf("cp: cannot create directory %s\n", target);
        return 1;
    }

    if (!target_root_known) {
        target_root = target_info;
        target_root_known = 1;
    }

    cp_level_t* level = malloc(sizeof(cp_level_t));

    if (!level) {
        puts("cp: out of memory\n");
        return 1;
    }

    int32_t dd = dirio_open(source);

    if (dd < 0) {
        printf("cp: cannot open directory %s\n", source);
        free(level);
        return 1;
    }

    int status = 0;

    while (dirio_read(dd, &level->entry) == 0) {
        const char* name = level->entry.name;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        if (cp_join(source, name, level->source) != 0 || cp_join(target, name, level->target) != 0) {
            printf("cp: path too long: %s/%s\n", source, name);
            status = 1;
            continue;
        }

        fileinfo_t info;

        if (fsio_stat(level->source, &info) != 0) {
            printf("cp: cannot access %s\n", level->source);
            status = 1;
            continue;
        }

        switch (info.type) {
            case FILE_TYPE_DIRECTORY:
                if (cp_same_file(&info, &target_root)) {
                    printf("cp: cannot copy %s into itself\n", level->source);
                    status = 1;
                    break;
                }

                if (cp_tree(level->source, level->target, depth + 1) != 0) {
                    status = 1;
                }

                break;
            case FILE_TYPE_FILE:
                if (cp_file(level->source, &info, level->target) != 0) {
                    status = 1;
                }

                break;
            default:
                // There is no way to read where a link points, so it cannot be recreated.
                printf("cp: skipping %s, not a regular file\n", level->source);
                status = 1;
                break;
        }
    }

    dirio_close(dd);
    free(level);

    return status;
}
