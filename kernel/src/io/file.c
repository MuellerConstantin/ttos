#include <io/file.h>
#include <fs/mount.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

/**
 * Resolve the directory that holds the last component of a path and report that component. Creating
 * and removing an entry are both operations on the directory the entry lives in rather than on the
 * entry itself, so both start here.
 *
 * @param name Receives a pointer into relative_path at the last component.
 * @return The directory node, to be released with file_release_directory, or NULL if the path in
 *         front of the last component does not resolve.
 */
static vfs_node_t* file_parent_directory(vfs_filesystem_t* mountpoint, char* relative_path, char** name) {
    char* separator = NULL;

    for(char* cursor = relative_path; *cursor != '\0'; cursor++) {
        if(*cursor == '/') {
            separator = cursor;
        }
    }

    if(separator == NULL) {
        *name = relative_path;
        return mountpoint->root;
    }

    size_t length = separator - relative_path;

    char* parent = (char*) kmalloc(length + 1);

    if(!parent) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    memcpy(parent, relative_path, length);
    parent[length] = '\0';

    vfs_node_t* directory = vfs_findpath(mountpoint->root, parent);

    kfree(parent);

    *name = separator + 1;

    return directory;
}

/**
 * Release a directory obtained from file_parent_directory. findpath hands out a fresh node for
 * anything below the root, but the root itself belongs to the mount point and must not be freed
 * with it.
 */
static void file_release_directory(vfs_filesystem_t* mountpoint, vfs_node_t* directory) {
    if(directory != NULL && directory != mountpoint->root) {
        kfree(directory);
    }
}

/**
 * Create a file at a path that does not exist yet.
 *
 * @return The node of the created file or NULL on error.
 */
static vfs_node_t* file_create(vfs_filesystem_t* mountpoint, char* relative_path, uint32_t permissions) {
    char* name;
    vfs_node_t* directory = file_parent_directory(mountpoint, relative_path, &name);

    if(!directory || *name == '\0') {
        file_release_directory(mountpoint, directory);
        return NULL;
    }

    int32_t result = vfs_create(directory, name, permissions);

    file_release_directory(mountpoint, directory);

    if(result != 0) {
        return NULL;
    }

    return vfs_findpath(mountpoint->root, relative_path);
}

/**
 * Remove an entry from the directory that holds it.
 *
 * Note that nothing keeps an entry alive while a file descriptor still refers to it: unlike a full
 * unix, removing a file that is still open frees its inode right away.
 *
 * @param directory True to remove a directory, false to remove a file.
 * @return 0 on success or -1 on error.
 */
static int32_t file_remove(char* path, bool directory) {
    if(!vfs_is_abs_path(path)) {
        return -1;
    }

    vfs_filesystem_t* mountpoint = (vfs_filesystem_t*) mnt_get_mountpoint(path);

    if(!mountpoint || !mountpoint->root) {
        return -1;
    }

    char* name;
    vfs_node_t* parent = file_parent_directory(mountpoint, path + 3, &name);

    if(!parent || *name == '\0') {
        file_release_directory(mountpoint, parent);
        return -1;
    }

    int32_t result = directory ? vfs_rmdir(parent, name) : vfs_unlink(parent, name);

    file_release_directory(mountpoint, parent);

    return result;
}

int32_t file_unlink(char* path) {
    return file_remove(path, false);
}

int32_t file_rmdir(char* path) {
    return file_remove(path, true);
}

file_descriptor_t* file_open(char* path, uint32_t flags, uint32_t permissions) {
    if(!vfs_is_abs_path(path)) {
        return NULL;
    }

    vfs_filesystem_t* mountpoint = (vfs_filesystem_t*) mnt_get_mountpoint(path);

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

    vfs_filesystem_t* mountpoint = (vfs_filesystem_t*) mnt_get_mountpoint(path);

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
