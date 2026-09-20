#include <devio.h>
#include <volio.h>
#include <mntio.h>
#include <fsio.h>
#include <dirio.h>
#include <proc.h>
#include <termio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ttos/syscall.h>

/*
 * Installs the running system onto a disk: one partition over the whole disk,
 * an ext2 file system labelled as the system volume, a copy of the system
 * tree and GRUB in front of the partition. Partitioning and formatting are
 * left to mkpart and mkfs; the copy and the bootloader are done here.
 */

/*
 * Copied in chunks of this size. The buffer is static: the initial user stack
 * is a single page and shared with the arguments and the environment.
 */
#define INSTALL_BUFFER_SIZE 4096

/** Deepest directory nesting the copy follows, see cp. */
#define INSTALL_MAX_DEPTH 32

/** Drive letters the copy of the tree may be mounted to while it is written. */
#define INSTALL_FIRST_DRIVE 'D'
#define INSTALL_LAST_DRIVE 'Z'

/** Where the system tree keeps the bootloader images, relative to its root. */
#define INSTALL_BOOT_IMAGE "boot/grub/i386-pc/boot.img"
#define INSTALL_CORE_IMAGE "boot/grub/i386-pc/core.img"

/** Programs the installer runs, relative to the root of the system tree. */
#define INSTALL_MKPART "bin/mkpart.elf"
#define INSTALL_MKFS "bin/mkfs.elf"

/** Widest answer read from the terminal. */
#define INSTALL_ANSWER_LENGTH 64

/** Storage devices offered as an install target. */
#define INSTALL_MAX_TARGETS 16

static char buffer[INSTALL_BUFFER_SIZE];

/** The paths and the directory entry one level of the copy works with. */
typedef struct {
    char source[PATH_MAX];
    char target[PATH_MAX];
    dirent_t entry;
} install_level_t;

/** A storage device the system can be installed onto. */
typedef struct {
    char id[sizeof(((devinfo_t*) 0)->id)];
    char name[sizeof(((devinfo_t*) 0)->name)];
    uint32_t size;
    uint32_t sector_size;
} install_target_t;

static int install_find_mounted(const char* label, char* drive);
static int install_find_system(char* drive);
static int install_device_mounted(const char* device_id);
static int install_list_targets(install_target_t* targets, int max);
static int install_choose_target(install_target_t* target);
static int install_confirm(const install_target_t* target);
static int install_run(const char* root, const char* program, char* const argv[]);
static int install_partition(const char* root, const install_target_t* target);
static int install_find_volume(const char* device_id, char* volume_id);
static int install_format(const char* root, const char* volume_id);
static int install_mount(const char* volume_id, char* drive);
static int install_join(const char* directory, const char* name, char* out);
static int install_copy_file(const char* source, const fileinfo_t* source_info, const char* target);
static int install_copy_tree(const char* source, const char* target, int depth);
static int install_bootloader(const char* root, const install_target_t* target);

int main(int argc, char** argv) {
    if (argc > 2) {
        puts("usage: install [<device>]\n");
        return 1;
    }

    char system_drive;

    if (install_find_system(&system_drive) != 0) {
        puts("install: the system volume is not mounted\n");
        return 1;
    }

    char root[4];

    sprintf(root, "%c:/", system_drive);

    install_target_t target;

    if (argc == 2) {
        install_target_t targets[INSTALL_MAX_TARGETS];
        int count = install_list_targets(targets, INSTALL_MAX_TARGETS);
        int found = 0;

        for (int index = 0; index < count; index++) {
            if (strcmp(targets[index].id, argv[1]) == 0) {
                target = targets[index];
                found = 1;
            }
        }

        if (!found) {
            printf("install: %s is not a disk the system can be installed on\n", argv[1]);
            return 1;
        }
    } else if (install_choose_target(&target) != 0) {
        return 1;
    }

    if (!install_confirm(&target)) {
        puts("install: aborted\n");
        return 1;
    }

    if (install_partition(root, &target) != 0) {
        return 1;
    }

    char volume_id[sizeof(((volinfo_t*) 0)->id)];

    if (install_find_volume(target.id, volume_id) != 0) {
        printf("install: no volume found on %s after partitioning\n", target.id);
        return 1;
    }

    if (install_format(root, volume_id) != 0) {
        return 1;
    }

    char drive;

    if (install_mount(volume_id, &drive) != 0) {
        printf("install: cannot mount %s\n", volume_id);
        return 1;
    }

    char target_root[4];

    sprintf(target_root, "%c:/", drive);

    printf("copying %s to %s\n", root, target_root);

    int status = install_copy_tree(root, target_root, 0);

    if (mntio_unmount(drive) != UNMOUNT_OK) {
        printf("install: cannot unmount %c:\n", drive);
        return 1;
    }

    if (status != 0) {
        puts("install: the copy is incomplete\n");
        return 1;
    }

    if (install_bootloader(root, &target) != 0) {
        return 1;
    }

    printf("installed on %s, reboot from it to start the installed system\n", target.id);

    return 0;
}

/** Finds the drive a mounted volume carrying the label sits on. Returns 0 when one was found. */
static int install_find_mounted(const char* label, char* drive) {
    volinfo_t volume;

    for (uint32_t index = 0; volio_list(index, &volume) == 0; index++) {
        if (strcmp(volume.label, label) != 0) {
            continue;
        }

        mntinfo_t mount;

        for (uint32_t position = 0; mntio_list(position, &mount) == 0; position++) {
            if (strcmp(mount.volume_id, volume.id) == 0) {
                *drive = mount.drive;
                return 0;
            }
        }
    }

    return -1;
}

/*
 * Finds the drive the running system is mounted to. The live medium is
 * preferred the way init prefers it, so a live session copies the medium it
 * was booted from even when an installed disk is mounted as well.
 */
static int install_find_system(char* drive) {
    if (install_find_mounted(TTOS_LIVE_LABEL, drive) == 0) {
        return 0;
    }

    return install_find_mounted(TTOS_SYSTEM_LABEL, drive);
}

/** Whether any volume of a device is mounted. Such a device is in use and cannot be rescanned. */
static int install_device_mounted(const char* device_id) {
    volinfo_t volume;

    for (uint32_t index = 0; volio_list(index, &volume) == 0; index++) {
        if (strcmp(volume.device_id, device_id) != 0) {
            continue;
        }

        mntinfo_t mount;

        for (uint32_t position = 0; mntio_list(position, &mount) == 0; position++) {
            if (strcmp(mount.volume_id, volume.id) == 0) {
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Collects the disks the system can be installed onto: storage devices with
 * addressable sectors that carry nothing mounted, which rules out the media
 * the running system lives on.
 */
static int install_list_targets(install_target_t* targets, int max) {
    devinfo_t device;
    int count = 0;

    for (uint32_t index = 0; count < max && devio_list(index, &device) == 0; index++) {
        if (device.type != DEVICE_TYPE_STORAGE) {
            continue;
        }

        storageinfo_t storage;

        if (devio_get_storageinfo(device.id, &storage) != 0 || storage.sector_size == 0) {
            continue;
        }

        if (install_device_mounted(device.id)) {
            continue;
        }

        strcpy(targets[count].id, device.id);
        strcpy(targets[count].name, device.name);
        targets[count].size = storage.size;
        targets[count].sector_size = storage.sector_size;
        count++;
    }

    return count;
}

/** Lists the possible targets and lets the user pick one by number. */
static int install_choose_target(install_target_t* target) {
    install_target_t targets[INSTALL_MAX_TARGETS];
    int count = install_list_targets(targets, INSTALL_MAX_TARGETS);

    if (count == 0) {
        puts("install: no disk to install on\n");
        return -1;
    }

    puts("Disks the system can be installed on:\n\n");

    for (int index = 0; index < count; index++) {
        char size[16];

        sizetoa(targets[index].size, size);
        printf("  %d) %-8s%6s  %s\n", index + 1, targets[index].id, size, targets[index].name);
    }

    char answer[INSTALL_ANSWER_LENGTH];

    for (;;) {
        printf("\nDisk to install on [1-%d], or q to quit: ", count);
        gets(answer);

        if (strcmp(answer, "q") == 0) {
            return -1;
        }

        char* end;
        unsigned long choice = strtoul(answer, &end, 10);

        if (end != answer && *end == '\0' && choice >= 1 && choice <= (unsigned long) count) {
            *target = targets[choice - 1];
            return 0;
        }
    }
}

/** Asks for the go-ahead. Everything on the disk is lost from here on. */
static int install_confirm(const install_target_t* target) {
    char answer[INSTALL_ANSWER_LENGTH];

    printf("\nAll data on %s (%s) will be lost. Continue? [y/N] ", target->id, target->name);
    gets(answer);

    return strcmp(answer, "y") == 0 || strcmp(answer, "Y") == 0;
}

/*
 * Runs a program from the system tree and reports whether it succeeded. The
 * program prints its own progress and failures; only starting it and its
 * exit status are judged here.
 */
static int install_run(const char* root, const char* program, char* const argv[]) {
    char path[PATH_MAX];

    if (install_join(root, program, path) != 0) {
        puts("install: path too long\n");
        return -1;
    }

    pid_t pid = spawn(path, argv, SPAWN_FOREGROUND);

    if (pid < 0) {
        printf("install: cannot run %s\n", path);
        return -1;
    }

    int status;

    wait(pid, &status, 0);
    termio_set_foreground(0);

    if (status != 0) {
        printf("install: %s failed (%d)\n", path, status);
        return -1;
    }

    return 0;
}

/** One partition over the whole disk, marked active so the master boot record boots it. */
static int install_partition(const char* root, const install_target_t* target) {
    char* argv[] = { INSTALL_MKPART, "-f", "-b", "1", (char*) target->id, "rest", NULL };

    printf("partitioning %s\n", target->id);

    return install_run(root, INSTALL_MKPART, argv);
}

/*
 * Finds the volume mkpart left on the device. Its id is new: a rescan hands
 * out fresh ids, so the volume is found by its device rather than remembered.
 */
static int install_find_volume(const char* device_id, char* volume_id) {
    volinfo_t volume;

    for (uint32_t index = 0; volio_list(index, &volume) == 0; index++) {
        if (strcmp(volume.device_id, device_id) == 0) {
            strcpy(volume_id, volume.id);
            return 0;
        }
    }

    return -1;
}

/** Formats the volume with the label init looks for on an installed disk. */
static int install_format(const char* root, const char* volume_id) {
    char* argv[] = { INSTALL_MKFS, (char*) volume_id, TTOS_SYSTEM_LABEL, NULL };

    printf("formatting %s\n", volume_id);

    return install_run(root, INSTALL_MKFS, argv);
}

/** Mounts the volume to the first free drive letter and reports which one. */
static int install_mount(const char* volume_id, char* drive) {
    for (char letter = INSTALL_FIRST_DRIVE; letter <= INSTALL_LAST_DRIVE; letter++) {
        int32_t status = mntio_mount(letter, volume_id);

        if (status == MOUNT_OK) {
            *drive = letter;
            return 0;
        }

        if (status != MOUNT_ERR_IN_USE) {
            return -1;
        }
    }

    return -1;
}

/** Writes directory/name into out. Fails if the result does not fit. */
static int install_join(const char* directory, const char* name, char* out) {
    size_t length = strlen(directory);

    if (length + 1 + strlen(name) >= PATH_MAX) {
        return -1;
    }

    strcpy(out, directory);

    if (length > 0 && out[length - 1] != '/') {
        out[length++] = '/';
        out[length] = '\0';
    }

    strcat(out, name);

    return 0;
}

/** Copies one file. Returns 0 on success or 1 after reporting the failure. */
static int install_copy_file(const char* source, const fileinfo_t* source_info, const char* target) {
    printf("  %s\n", target);

    int32_t in = fsio_open(source, FSIO_RDONLY, 0);

    if (in < 0) {
        printf("install: cannot open %s\n", source);
        return 1;
    }

    int32_t out = fsio_open(target, FSIO_WRONLY | FSIO_CREAT | FSIO_TRUNC, source_info->permissions);

    if (out < 0) {
        printf("install: cannot create %s\n", target);
        fsio_close(in);
        return 1;
    }

    int status = 0;
    int32_t bytes_read;

    while ((bytes_read = fsio_read(in, buffer, sizeof(buffer))) > 0) {
        // A short write is what a full file system looks like from here.
        if (fsio_write(out, buffer, bytes_read) != bytes_read) {
            printf("install: write failed: %s\n", target);
            status = 1;
            break;
        }
    }

    if (bytes_read < 0) {
        printf("install: read failed: %s\n", source);
        status = 1;
    }

    fsio_close(out);
    fsio_close(in);

    return status;
}

/*
 * Copies the directory source into target, creating target if it does not
 * exist. The roots of both trees exist already, and so does lost+found on a
 * freshly made file system. Failures are reported and the copy goes on with
 * the next entry; the return value tells whether everything succeeded.
 */
static int install_copy_tree(const char* source, const char* target, int depth) {
    if (depth >= INSTALL_MAX_DEPTH) {
        printf("install: %s is nested too deep\n", source);
        return 1;
    }

    fileinfo_t target_info;

    if (fsio_stat(target, &target_info) == 0) {
        if (target_info.type != FILE_TYPE_DIRECTORY) {
            printf("install: %s is not a directory\n", target);
            return 1;
        }
    } else if (fsio_mkdir(target, 0755) != 0) {
        printf("install: cannot create directory %s\n", target);
        return 1;
    }

    install_level_t* level = malloc(sizeof(install_level_t));

    if (!level) {
        puts("install: out of memory\n");
        return 1;
    }

    int32_t dd = dirio_open(source);

    if (dd < 0) {
        printf("install: cannot open directory %s\n", source);
        free(level);
        return 1;
    }

    int status = 0;

    while (dirio_read(dd, &level->entry) == 0) {
        const char* name = level->entry.name;

        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }

        if (install_join(source, name, level->source) != 0 || install_join(target, name, level->target) != 0) {
            printf("install: path too long: %s/%s\n", source, name);
            status = 1;
            continue;
        }

        fileinfo_t info;

        if (fsio_stat(level->source, &info) != 0) {
            printf("install: cannot access %s\n", level->source);
            status = 1;
            continue;
        }

        switch (info.type) {
            case FILE_TYPE_DIRECTORY:
                if (install_copy_tree(level->source, level->target, depth + 1) != 0) {
                    status = 1;
                }

                break;
            case FILE_TYPE_FILE:
                if (install_copy_file(level->source, &info, level->target) != 0) {
                    status = 1;
                }

                break;
            default:
                // There is no way to read where a link points, so it cannot be recreated.
                printf("install: skipping %s, not a regular file\n", level->source);
                status = 1;
                break;
        }
    }

    dirio_close(dd);
    free(level);

    return status;
}

/*
 * Writes GRUB onto the disk the way the build does: the code of boot.img into
 * the master boot record, in front of the partition table mkpart wrote there,
 * and core.img into the gap between the master boot record and the
 * partition. core.img has its location on the disk compiled in, so both go
 * where the build configuration says and nowhere else.
 */
static int install_bootloader(const char* root, const install_target_t* target) {
    char path[PATH_MAX];
    fileinfo_t info;

    if (install_join(root, INSTALL_CORE_IMAGE, path) != 0 || fsio_stat(path, &info) != 0) {
        printf("install: cannot access %s\n", path);
        return -1;
    }

    size_t gap = (TTOS_PARTITION_START - TTOS_CORE_SECTOR) * target->sector_size;

    if (info.size > gap) {
        printf("install: %s does not fit in front of the partition\n", path);
        return -1;
    }

    printf("writing %s\n", path);

    int32_t in = fsio_open(path, FSIO_RDONLY, 0);

    if (in < 0) {
        printf("install: cannot open %s\n", path);
        return -1;
    }

    size_t offset = TTOS_CORE_SECTOR * target->sector_size;
    int32_t bytes_read;

    while ((bytes_read = fsio_read(in, buffer, sizeof(buffer))) > 0) {
        if (devio_write(target->id, offset, buffer, bytes_read) != bytes_read) {
            printf("install: cannot write %s to %s\n", path, target->id);
            fsio_close(in);
            return -1;
        }

        offset += bytes_read;
    }

    fsio_close(in);

    if (bytes_read < 0) {
        printf("install: read failed: %s\n", path);
        return -1;
    }

    if (install_join(root, INSTALL_BOOT_IMAGE, path) != 0) {
        puts("install: path too long\n");
        return -1;
    }

    printf("writing %s\n", path);

    in = fsio_open(path, FSIO_RDONLY, 0);

    if (in < 0) {
        printf("install: cannot open %s\n", path);
        return -1;
    }

    bytes_read = fsio_read(in, buffer, TTOS_MBR_CODE_SIZE);
    fsio_close(in);

    if (bytes_read != TTOS_MBR_CODE_SIZE) {
        printf("install: %s is too short\n", path);
        return -1;
    }

    if (devio_write(target->id, 0, buffer, TTOS_MBR_CODE_SIZE) != TTOS_MBR_CODE_SIZE) {
        printf("install: cannot write %s to %s\n", path, target->id);
        return -1;
    }

    return 0;
}
