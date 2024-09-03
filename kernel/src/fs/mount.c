#include <fs/mount.h>

static mnt_volume_t* mnt_volume_mountpoints[FS_VOLUME_MAX_MOUNTPOINTS];

static int32_t mnt_get_drive_index(char drive);

int32_t mnt_volume_mount(char drive, mnt_volume_t* volume) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(mnt_volume_mountpoints[index]) {
        return -1;
    }

    if(volume->operations->mount(volume) != 0) {
        return -1;
    }

    mnt_volume_mountpoints[index] = volume;

    return 0;
}

int32_t mnt_volume_unmount(char drive) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(!mnt_volume_mountpoints[index]) {
        return -1;
    }

    if(mnt_volume_mountpoints[index]->operations->unmount(mnt_volume_mountpoints[index]) != 0) {
        return -1;
    }

    mnt_volume_mountpoints[index] = NULL;

    return 0;
}

mnt_volume_t* mnt_get_mountpoint(char* path) {
    if(strlen(path) < 3) {
        return NULL;
    }

    int32_t index = mnt_get_drive_index(path[0]);

    if(index == -1) {
        return NULL;
    }

    return mnt_volume_mountpoints[index];
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
