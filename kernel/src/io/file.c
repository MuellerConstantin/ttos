#include <io/file.h>
#include <fs/mount.h>

file_descriptor_t file_descriptors[MAX_FILE_DESCRIPTORS];

static int32_t file_get_free_descriptor();

int32_t file_open(char* path, uint32_t flags) {
    if(!vfs_is_abs_path(path)) {
        return -1;
    }

    mnt_mountpoint_t* mountpoint = mnt_get_mountpoint(path);

    if(!mountpoint) {
        return -1;
    }

    char* relative_path = path + 3;

    vfs_node_t* node = vfs_findpath(mountpoint->root, relative_path);

    if(!node) {
        kfree(node);
        return -1;
    }

    int32_t fd = file_get_free_descriptor();

    if(fd < 0) {
        kfree(node);
        return -1;
    }

    file_descriptors[fd].node = node;
    file_descriptors[fd].size = node->length;
    file_descriptors[fd].offset = 0;
    file_descriptors[fd].flags = flags;

    if(vfs_open(node) != 0) {
        file_descriptors[fd].node = NULL;
        kfree(node);
        return -1;
    }

    return fd;
}

int32_t file_close(int32_t fd) {
    if(fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1;
    }

    if(!file_descriptors[fd].node) {
        return -1;
    }

    if(vfs_close(file_descriptors[fd].node) != 0) {
        return -1;
    }

    kfree(file_descriptors[fd].node);
    file_descriptors[fd].node = NULL;

    return 0;
}

int32_t file_read(int32_t fd, void* buffer, size_t size) {
    if(fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1;
    }

    if(!file_descriptors[fd].node) {
        return -1;
    }

    if(!(file_descriptors[fd].flags & FILE_MODE_R)) {
        return -1;
    }

    if(file_descriptors[fd].offset + size > file_descriptors[fd].size) {
        size = file_descriptors[fd].size - file_descriptors[fd].offset;
    }

    int32_t bytes_read = vfs_read(file_descriptors[fd].node, file_descriptors[fd].offset, size, buffer);

    if(bytes_read < 0) {
        return -1;
    }

    file_descriptors[fd].offset += bytes_read;

    return bytes_read;
}

int32_t file_write(int32_t fd, void* buffer, size_t size) {
    if(fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1;
    }

    if(!file_descriptors[fd].node) {
        return -1;
    }

    if(!(file_descriptors[fd].flags & FILE_MODE_W)) {
        return -1;
    }

    if(file_descriptors[fd].offset + size > file_descriptors[fd].size) {
        size = file_descriptors[fd].size - file_descriptors[fd].offset;
    }

    int32_t bytes_written = vfs_write(file_descriptors[fd].node, file_descriptors[fd].offset, size, buffer);

    if(bytes_written < 0) {
        return -1;
    }

    file_descriptors[fd].offset += bytes_written;

    return bytes_written;
}

int32_t file_seek(int32_t fd, int32_t offset, int32_t whence) {
    if(fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1;
    }

    if(!file_descriptors[fd].node) {
        return -1;
    }

    switch(whence) {
        case FILE_SEEK_CUR:
            file_descriptors[fd].offset += offset;
            break;
        case FILE_SEEK_BEGIN:
            file_descriptors[fd].offset = offset;
            break;
        case FILE_SEEK_END:
            file_descriptors[fd].offset = file_descriptors[fd].size + offset;
            break;
        default:
            return -1;
    }

    if(file_descriptors[fd].offset < 0) {
        file_descriptors[fd].offset = 0;
    }

    if(file_descriptors[fd].offset > file_descriptors[fd].size) {
        file_descriptors[fd].offset = file_descriptors[fd].size;
    }

    return 0;
}

size_t file_size(const char* path) {
    if(!vfs_is_abs_path(path)) {
        return -1;
    }

    mnt_mountpoint_t* mountpoint = mnt_get_mountpoint(path);

    if(!mountpoint) {
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

    
    size_t size = node->length;

    kfree(node);

    return size;
}

int32_t file_stat(const char* path, file_stat_t* stat) {
    if(!vfs_is_abs_path(path)) {
        return -1;
    }

    mnt_mountpoint_t* mountpoint = mnt_get_mountpoint(path);

    if(!mountpoint) {
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

static int32_t file_get_free_descriptor() {
    for(size_t index = 0; index < MAX_FILE_DESCRIPTORS; index++) {
        if(!file_descriptors[index].node) {
            return index;
        }
    }

    return -1;
}
