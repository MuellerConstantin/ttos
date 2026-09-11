#include <fs/mount.h>
#include <fs/initfs.h>
#include <fs/ext2.h>

static vfs_filesystem_t* mnt_mountpoints[FS_VOLUME_MAX_MOUNTPOINTS];

static int32_t mnt_get_drive_index(char drive);

int32_t mnt_volume_mount(char drive, volume_t* volume) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(mnt_mountpoints[index]) {
        return -1;
    }

    /*
     * A volume may hang off one drive only. A second file system instance on
     * the same volume would keep its own copy of the metadata and write it
     * back over the other's.
     */
    if(mnt_get_volume_drive(volume) != 0) {
        return -1;
    }

    // Probe for the file system
    if(initfs_probe(volume)) {
        mnt_mountpoints[index] = initfs_init(volume);

        if(mnt_mountpoints[index]->operations->mount(mnt_mountpoints[index]) != 0) {
            mnt_mountpoints[index]->operations->unmount(mnt_mountpoints[index]);
            mnt_mountpoints[index] = NULL;
            return -1;
        }

        return 0;
    }

    if(ext2_probe(volume)) {
        mnt_mountpoints[index] = ext2_init(volume);

        if(mnt_mountpoints[index]->operations->mount(mnt_mountpoints[index]) != 0) {
            mnt_mountpoints[index]->operations->unmount(mnt_mountpoints[index]);
            mnt_mountpoints[index] = NULL;
            return -1;
        }

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

const vfs_filesystem_t* mnt_get_mountpoint(char* path) {
    if(strlen(path) < 3) {
        return NULL;
    }

    int32_t index = mnt_get_drive_index(path[0]);

    if(index == -1) {
        return NULL;
    }

    return mnt_mountpoints[index];
}

const vfs_filesystem_t* mnt_get_drive(char drive) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return NULL;
    }

    return mnt_mountpoints[index];
}

char mnt_get_volume_drive(const volume_t* volume) {
    for(char drive = DRIVE_A; drive <= DRIVE_Z; drive++) {
        const vfs_filesystem_t* filesystem = mnt_mountpoints[drive - DRIVE_A];

        if(filesystem && filesystem->volume == volume) {
            return drive;
        }
    }

    return 0;
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
