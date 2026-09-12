#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proc.h>
#include <volio.h>
#include <mntio.h>

#define INIT_FIRST_DRIVE 'C'
#define INIT_LAST_DRIVE 'Z'

/** Where the shell looks for commands; the initrd holds the system programs. */
#define INIT_PATH "A:/"

static int init_is_mounted(const char* volume_id);
static char init_next_free_drive(void);
static void init_mount_volumes(void);

int main(void) {
    const char* shell_path = "A:/shell.elf";
    char* shell_argv[] = { (char*) shell_path, 0 };

    init_mount_volumes();

    // The environment every process descends from starts here.
    setenv("PATH", INIT_PATH, 1);

    /*
     * init is PID 1: it must never exit. Keep a shell running and respawn it if
     * it ever terminates. If init itself were to return, the kernel raises a
     * panic (see process_terminate).
     */
    for(;;) {
        int code = spawn(shell_path, shell_argv);

        printf("init: shell exited (%d), restarting\n", code);
    }

    return 0;
}

/*
 * Mounts every volume the system found and that is not mounted yet, the initial
 * ramdisk included: the kernel mounted it before this process existed, so it is
 * skipped by the same check as anything else. A volume that carries no known
 * file system is passed over; an empty disk is a normal state. The kernel logs
 * what ends up mounted.
 */
static void init_mount_volumes(void) {
    volinfo_t volume;

    for (uint32_t index = 0; volio_list(index, &volume) == 0; index++) {
        if (init_is_mounted(volume.id)) {
            continue;
        }

        char drive = init_next_free_drive();

        if (drive == 0) {
            return;
        }

        mntio_mount(drive, volume.id);
    }
}

static int init_is_mounted(const char* volume_id) {
    mntinfo_t mount;

    for (uint32_t index = 0; mntio_list(index, &mount) == 0; index++) {
        if (strcmp(mount.volume_id, volume_id) == 0) {
            return 1;
        }
    }

    return 0;
}

static char init_next_free_drive(void) {
    for (char drive = INIT_FIRST_DRIVE; drive <= INIT_LAST_DRIVE; drive++) {
        mntinfo_t mount;
        int taken = 0;

        for (uint32_t index = 0; mntio_list(index, &mount) == 0; index++) {
            if (mount.drive == drive) {
                taken = 1;
                break;
            }
        }

        if (!taken) {
            return drive;
        }
    }

    return 0;
}
