#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proc.h>
#include <volio.h>
#include <mntio.h>
#include <ttos/syscall.h>

/** The drive the system volume is mounted to. A: belongs to the initial ramdisk. */
#define INIT_SYSTEM_DRIVE 'C'

/** Where programs are looked for: the ramdisk holds the system programs, the system volume the rest. */
#define INIT_PATH_FORMAT "A:/;%c:/bin"

static int init_find_volume(const char* label, char* id);
static void init_mount_system(void);

int main(void) {
    const char* shell_path = "A:/shell.elf";
    char* shell_argv[] = { (char*) shell_path, 0 };

    init_mount_system();

    /*
     * init is PID 1: it must never exit. Keep a shell running and respawn it if
     * it ever terminates. Children whose parent has exited are handed to init
     * by the kernel, so the wait also collects those; only the shell is
     * restarted. If init itself were to return, the kernel raises a panic (see
     * process_exit).
     */
    pid_t shell = spawn(shell_path, shell_argv);

    for(;;) {
        int status = -1;
        pid_t pid = wait(-1, &status, 0);

        if(pid == shell || shell < 0) {
            printf("init: shell exited (%d), restarting\n", status);
            shell = spawn(shell_path, shell_argv);
        }
    }

    return 0;
}

/*
 * Mounts the system volume and publishes the environment every process
 * descends from. Volumes that are not the system are left alone; they are
 * mounted by hand when they are needed.
 */
static void init_mount_system(void) {
    char id[sizeof(((volinfo_t*) 0)->id)];
    char path[PATH_MAX];

    /*
     * The live medium is looked for first: it and an installed disk hold the
     * same tree, so a live session keeps running from the medium it was booted
     * from even when a system is installed on the disk.
     */
    if(init_find_volume(TTOS_LIVE_LABEL, id) != 0 && init_find_volume(TTOS_SYSTEM_LABEL, id) != 0) {
        setenv("PATH", "A:/", 1);
        return;
    }

    if(mntio_mount(INIT_SYSTEM_DRIVE, id) != MOUNT_OK) {
        setenv("PATH", "A:/", 1);
        return;
    }

    sprintf(path, INIT_PATH_FORMAT, INIT_SYSTEM_DRIVE);

    setenv("PATH", path, 1);
}

/** Finds the volume carrying a label and copies its id. Returns 0 when one was found. */
static int init_find_volume(const char* label, char* id) {
    volinfo_t volume;

    for(uint32_t index = 0; volio_list(index, &volume) == 0; index++) {
        if(strcmp(volume.label, label) == 0) {
            strcpy(id, volume.id);
            return 0;
        }
    }

    return -1;
}
