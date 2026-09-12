#include <fsio.h>
#include <stdio.h>
#include <string.h>
#include <ttos/syscall.h>

/*
 * Copied in chunks of this size. The buffer is static: the initial user stack
 * is a single page and shared with the arguments and the environment.
 */
#define CP_BUFFER_SIZE 4096

static char buffer[CP_BUFFER_SIZE];

/** Whether a path names an existing directory. */
static int cp_is_directory(const char* path) {
    fileinfo_t info;

    return fsio_stat(path, &info) == 0 && info.type == FILE_TYPE_DIRECTORY;
}

/*
 * Whether two paths name the same object. Volume and inode identify a file;
 * the type is compared as well, since the initial ramdisk numbers its root
 * and its first file alike.
 */
static int cp_same_file(const fileinfo_t* a, const fileinfo_t* b) {
    return a->type == b->type && a->inode == b->inode && strcmp(a->volume_id, b->volume_id) == 0;
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

/*
 * Builds the path the copy is written to. A destination that is a directory
 * receives a file of the source's name inside it; anything else is taken as
 * the file name itself.
 */
static int cp_target(const char* source, const char* destination, char* target) {
    if (!cp_is_directory(destination)) {
        if (strlen(destination) >= PATH_MAX) {
            return -1;
        }

        strcpy(target, destination);

        return 0;
    }

    const char* name = cp_basename(source);
    size_t length = strlen(destination);

    if (length + 1 + strlen(name) >= PATH_MAX) {
        return -1;
    }

    strcpy(target, destination);

    if (target[length - 1] != '/') {
        target[length++] = '/';
        target[length] = '\0';
    }

    strcat(target, name);

    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        puts("usage: cp <source> <destination>\n");
        return 1;
    }

    const char* source = argv[1];
    char target[PATH_MAX];
    fileinfo_t source_info;
    fileinfo_t target_info;

    if (fsio_stat(source, &source_info) != 0) {
        puts("cp: cannot open source\n");
        return 1;
    }

    if (source_info.type == FILE_TYPE_DIRECTORY) {
        puts("cp: source is a directory\n");
        return 1;
    }

    if (cp_target(source, argv[2], target) != 0) {
        puts("cp: path too long\n");
        return 1;
    }

    /*
     * Opening the target truncates it, so a target that is the source would be
     * emptied before a byte is read. An existing target is compared by identity,
     * which catches the same file under any two names.
     */
    if (fsio_stat(target, &target_info) == 0 && cp_same_file(&source_info, &target_info)) {
        puts("cp: source and destination are the same file\n");
        return 1;
    }

    int32_t in = fsio_open(source, FSIO_RDONLY, 0);

    if (in < 0) {
        puts("cp: cannot open source\n");
        return 1;
    }

    int32_t out = fsio_open(target, FSIO_WRONLY | FSIO_CREAT | FSIO_TRUNC, 0644);

    if (out < 0) {
        puts("cp: cannot open destination\n");
        fsio_close(in);
        return 1;
    }

    int status = 0;
    int32_t bytes_read;

    while ((bytes_read = fsio_read(in, buffer, sizeof(buffer))) > 0) {
        // A short write is what a full file system looks like from here.
        if (fsio_write(out, buffer, bytes_read) != bytes_read) {
            puts("cp: write failed\n");
            status = 1;
            break;
        }
    }

    if (bytes_read < 0) {
        puts("cp: read failed\n");
        status = 1;
    }

    fsio_close(out);
    fsio_close(in);

    return status;
}
