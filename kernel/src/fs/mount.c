#include <fs/mount.h>

static mnt_mountpoint_t* mnt_mountpoints[FS_VOLUME_MAX_MOUNTPOINTS];

static int32_t mnt_get_drive_index(char drive);

int32_t mnt_volume_mount(char drive, volume_t* volume) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(mnt_mountpoints[index]) {
        return -1;
    }

    // Probe for the file system
    if(initfs_probe(volume)) {
        mnt_mountpoints[index] = initfs_init(volume);
        return 0;
    }

    return -1;
}

int32_t mnt_volume_unmount(char drive) {
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

const mnt_mountpoint_t* mnt_get_mountpoint(char* path) {
    if(strlen(path) < 3) {
        return NULL;
    }

    int32_t index = mnt_get_drive_index(path[0]);

    if(index == -1) {
        return NULL;
    }

    return mnt_mountpoints[index];
}

const mnt_mountpoint_t* mnt_get_drive(char drive) {
    int32_t index = mnt_get_drive_index(drive);

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
