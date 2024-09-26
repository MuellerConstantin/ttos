#include <fs/mount.h>

static mnt_mountpoint_t* mnt_mountpoints[FS_VOLUME_MAX_MOUNTPOINTS];

static int32_t mnt_get_drive_index(char drive);

int32_t mnt_drive_mount(char drive, mnt_mountpoint_t* mountpoint) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(mnt_mountpoints[index]) {
        return -1;
    }

    if(mountpoint->operations->mount(mountpoint) != 0) {
        return -1;
    }

    mnt_mountpoints[index] = mountpoint;

    return 0;
}

int32_t mnt_drive_unmount(char drive) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(!mnt_mountpoints[index]) {
        return -1;
    }

    if(mnt_mountpoints[index]->operations->unmount(mnt_mountpoints[index]) != 0) {
        return -1;
    }

    mnt_mountpoints[index] = NULL;

    return 0;
}

mnt_mountpoint_t* mnt_get_mountpoint(char* path) {
    if(strlen(path) < 3) {
        return NULL;
    }

    int32_t index = mnt_get_drive_index(path[0]);

    if(index == -1) {
        return NULL;
    }

    return mnt_mountpoints[index];
}

static int32_t mnt_get_drive_index(char drive) {
    if(drive >= 'a' && drive <= 'z') {
        return drive - 'a';
    }

    if(drive >= 'A' && drive <= 'Z') {
        return drive - 'A';
    }

    return -1;
}
