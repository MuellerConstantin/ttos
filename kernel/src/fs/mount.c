#include <fs/mount.h>
#include <fs/initfs.h>
#include <fs/ext2.h>
#include <memory/kheap.h>
#include <system/kmessage.h>
#include <system/kpanic.h>

static vfs_filesystem_t* mnt_mountpoints[FS_VOLUME_MAX_MOUNTPOINTS];
static bool mnt_locked[FS_VOLUME_MAX_MOUNTPOINTS];

static int32_t mnt_get_drive_index(char drive);
static void mnt_log(const char* action, char drive, const char* volume_id, const char* type);

int32_t mnt_probe_volume(volume_t* volume, char* type, char* label) {
    type[0] = '\0';
    label[0] = '\0';

    if(!volume) {
        return -1;
    }

    if(initfs_probe(volume)) {
        strcpy(type, "initfs");
        return 0;
    }

    if(ext2_probe(volume)) {
        strcpy(type, "ext2");
        ext2_label(volume, label, MNT_LABEL_LENGTH);
        return 0;
    }

    return -1;
}

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

        mnt_log("Mounted", DRIVE_A + index, volume->id, mnt_mountpoints[index]->type);

        return 0;
    }

    if(ext2_probe(volume)) {
        mnt_mountpoints[index] = ext2_init(volume);

        if(mnt_mountpoints[index]->operations->mount(mnt_mountpoints[index]) != 0) {
            mnt_mountpoints[index]->operations->unmount(mnt_mountpoints[index]);
            mnt_mountpoints[index] = NULL;
            return -1;
        }

        mnt_log("Mounted", DRIVE_A + index, volume->id, mnt_mountpoints[index]->type);

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

    if(mnt_locked[index]) {
        return -1;
    }

    /*
     * The file system object is gone after the unmount; the volume and the
     * type literal outlive it.
     */
    const char* volume_id = mnt_mountpoints[index]->volume->id;
    const char* type = mnt_mountpoints[index]->type;

    if(mnt_mountpoints[index]->operations->unmount(mnt_mountpoints[index]) != 0) {
        return -1;
    }

    mnt_mountpoints[index] = NULL;

    mnt_log("Unmounted", DRIVE_A + index, volume_id, type);

    return 0;
}

int32_t mnt_drive_lock(char drive) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return -1;
    }

    if(!mnt_mountpoints[index]) {
        return -1;
    }

    mnt_locked[index] = true;

    return 0;
}

bool mnt_drive_is_locked(char drive) {
    int32_t index = mnt_get_drive_index(drive);

    if(index == -1) {
        return false;
    }

    return mnt_locked[index];
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

/*
 * The log keeps the message pointer rather than a copy, so the buffer is taken
 * from the heap and never released.
 */
static void mnt_log(const char* action, char drive, const char* volume_id, const char* type) {
    char* message = (char*) kmalloc(MNT_MESSAGE_LENGTH);

    if(!message) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strfmt(message, "mount: %s volume %s on %c: as %s", action, volume_id, drive, type);

    kmessage(KMESSAGE_LEVEL_INFO, message);
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
