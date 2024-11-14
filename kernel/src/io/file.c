#include <io/file.h>
#include <fs/mount.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

file_descriptor_t* file_open(char* path, uint32_t flags) {
    if(!vfs_is_abs_path(path)) {
        return NULL;
    }

    vfs_filesystem_t* mountpoint = mnt_get_mountpoint(path);

    if(!mountpoint || !mountpoint->root) {
        return NULL;
    }

    char* relative_path = path + 3;

    vfs_node_t* node = vfs_findpath(mountpoint->root, relative_path);

    if(!node) {
        kfree(node);
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
        kfree(node->name);
        kfree(node);
        return NULL;
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

    if(fd->offset + size > fd->size) {
        size = fd->size - fd->offset;
    }

    int32_t bytes_written = vfs_write(fd->node, fd->offset, size, buffer);

    if(bytes_written < 0) {
        return -1;
    }

    fd->offset += bytes_written;

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
