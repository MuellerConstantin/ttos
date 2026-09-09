#include <io/file.h>
#include <fs/mount.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

/**
 * Create a file at a path that does not exist yet. Splits the path into the directory that will
 * hold the new entry and the name of the entry itself, because a file system expects the create
 * operation on the directory rather than on the file.
 *
 * @return The node of the created file or NULL on error.
 */
static vfs_node_t* file_create(vfs_filesystem_t* mountpoint, char* relative_path, uint32_t permissions) {
    char* separator = NULL;

    for(char* cursor = relative_path; *cursor != '\0'; cursor++) {
        if(*cursor == '/') {
            separator = cursor;
        }
    }

    vfs_node_t* directory = mountpoint->root;
    char* name = relative_path;

    if(separator != NULL) {
        size_t length = separator - relative_path;

        char* parent = (char*) kmalloc(length + 1);

        if(!parent) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        memcpy(parent, relative_path, length);
        parent[length] = '\0';

        directory = vfs_findpath(mountpoint->root, parent);
        name = separator + 1;

        kfree(parent);
    }

    if(!directory || *name == '\0') {
        return NULL;
    }

    int32_t result = vfs_create(directory, name, permissions);

    // findpath hands out a fresh node for anything below the root; the root itself belongs to the
    // mount point and must not be freed with it.
    if(directory != mountpoint->root) {
        kfree(directory);
    }

    if(result != 0) {
        return NULL;
    }

    return vfs_findpath(mountpoint->root, relative_path);
}

file_descriptor_t* file_open(char* path, uint32_t flags, uint32_t permissions) {
    if(!vfs_is_abs_path(path)) {
        return NULL;
    }

    vfs_filesystem_t* mountpoint = mnt_get_mountpoint(path);

    if(!mountpoint || !mountpoint->root) {
        return NULL;
    }

    char* relative_path = path + 3;

    vfs_node_t* node = vfs_findpath(mountpoint->root, relative_path);

    if(!node && (flags & FILE_CREAT)) {
        node = file_create(mountpoint, relative_path, permissions);
    }

    if(!node) {
        return NULL;
    }

    file_descriptor_t* file_descriptor = (file_descriptor_t*) kmalloc(sizeof(file_descriptor_t));

    if(file_descriptor == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    file_descriptor->node = node;
    file_descriptor->size = node->length;
    file_descriptor->offset = 0;
    file_descriptor->flags = flags;

    if(vfs_open(node) != 0) {
        kfree(file_descriptor);
        kfree(node);
        return NULL;
    }

    // Discarding the previous contents needs the inode loaded, so it has to happen after the open.
    if(flags & FILE_TRUNC) {
        if(vfs_truncate(node, 0) != 0) {
            vfs_close(node);
            kfree(file_descriptor);
            kfree(node);
            return NULL;
        }

        file_descriptor->size = 0;
    }

    return file_descriptor;
}

int32_t file_close(file_descriptor_t* fd) {
    if(!fd) {
        return -1;
    }

    if(!fd->node) {
        return -1;
    }

    if(vfs_close(fd->node) != 0) {
        return -1;
    }

    kfree(fd);

    return 0;
}

int32_t file_read(file_descriptor_t* fd, void* buffer, size_t size) {
    if(!fd) {
        return -1;
    }

    if(!fd->node) {
        return -1;
    }

    if(!(fd->flags & FILE_RDONLY)) {
        return -1;
    }

    if(fd->offset + size > fd->size) {
        size = fd->size - fd->offset;
    }

    int32_t bytes_read = vfs_read(fd->node, fd->offset, size, buffer);

    if(bytes_read < 0) {
        return -1;
    }

    fd->offset += bytes_read;

    return bytes_read;
}

int32_t file_write(file_descriptor_t* fd, void* buffer, size_t size) {
    if(!fd) {
        return -1;
    }

    if(!fd->node) {
        return -1;
    }

    if(!(fd->flags & FILE_WRONLY)) {
        return -1;
    }

    // An appending descriptor always writes at the current end of the file, wherever an earlier
    // write or seek left the offset.
    if(fd->flags & FILE_APPEND) {
        fd->offset = fd->node->length;
    }

    int32_t bytes_written = vfs_write(fd->node, fd->offset, size, buffer);

    if(bytes_written < 0) {
        return -1;
    }

    fd->offset += bytes_written;

    // Writing past the end grows the file, so the size this descriptor reads and seeks against has
    // to follow along.
    if(fd->offset > fd->size) {
        fd->size = fd->offset;
    }

    return bytes_written;
}

int32_t file_seek(file_descriptor_t* fd, int32_t offset, int32_t whence) {
    if(!fd) {
        return -1;
    }

    if(!fd->node) {
        return -1;
    }

    switch(whence) {
        case FILE_SEEK_CUR:
            fd->offset += offset;
            break;
        case FILE_SEEK_BEGIN:
            fd->offset = offset;
            break;
        case FILE_SEEK_END:
            fd->offset = fd->size + offset;
            break;
        default:
            return -1;
    }

    if(fd->offset < 0) {
        fd->offset = 0;
    }

    if(fd->offset > fd->size) {
        fd->offset = fd->size;
    }

    return 0;
}

int32_t file_stat(const char* path, file_stat_t* stat) {
    if(!vfs_is_abs_path(path)) {
        return -1;
    }

    vfs_filesystem_t* mountpoint = mnt_get_mountpoint(path);

    if(!mountpoint || !mountpoint->root) {
        return -1;
    }

    char* relative_path = path + 3;

    vfs_node_t* node = vfs_findpath(mountpoint->root, relative_path);

    if(!node) {
        kfree(node);
        return -1;
    }

    if(node->type != VFS_FILE) {
        kfree(node);
        return -1;
    }

    stat->size = node->length;
    stat->uid = node->uid;
    stat->gid = node->gid;
    stat->permissions = node->permissions;

    kfree(node);

    return 0;
}
