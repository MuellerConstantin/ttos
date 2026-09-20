#include <system/syscall.h>
#include <arch/i386/isr.h>
#include <io/stream.h>
#include <io/file.h>
#include <io/dir.h>
#include <io/path.h>
#include <io/tty.h>
#include <device/device.h>
#include <device/volume.h>
#include <fs/mount.h>
#include <system/kmessage.h>
#include <system/timer.h>
#include <memory/kheap.h>
#include <util/generic_tree.h>
#include <util/linked_list.h>
#include <util/uuid.h>
#include <arch/i386/acpi.h>
#include <system/process.h>
#include <kernel.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <util/string.h>
#include <ttos/syscall.h>

static void syscall_handler(isr_cpu_state_t *state);

/**
 * Read syscall handler.
 * 
 * Syscall expects the following parameters:
 * 
 * - eax: Syscall number
 * 
 * - ebx: File descriptor
 * 
 * - ecx: Buffer
 * 
 * - edx: Size
 * 
 * Syscall returns the number of bytes read or -1 on error.
 * 
 * @param state The CPU state.
 * @return The number of bytes read or -1 on error.
 */
static int32_t syscall_read(isr_cpu_state_t *state);

/**
 * Write syscall handler.
 * 
 * Syscall expects the following parameters:
 * 
 * - eax: Syscall number
 * 
 * - ebx: File descriptor
 * 
 * - ecx: Buffer
 * 
 * - edx: Size
 * 
 * Syscall returns the number of bytes written or -1 on error.
 * 
 * @param state The CPU state.
 * @return The number of bytes written or -1 on error.
 */
static int32_t syscall_write(isr_cpu_state_t *state);

/**
 * Open syscall handler.
 * 
 * Syscall expects the following parameters:
 * 
 * - eax: Syscall number
 * 
 * - ebx: File name
 * 
 * - ecx: Flags
 * 
 * - edx: Mode
 * 
 * Syscall returns the file descriptor or -1 on error.
 * 
 * @param state The CPU state.
 */
static int32_t syscall_open(isr_cpu_state_t *state);

/**
 * Close syscall handler.
 * 
 * Syscall expects the following parameters:
 * 
 * - eax: Syscall number
 * 
 * - ebx: File descriptor
 * 
 * Syscall returns 0 on success or -1 on error.
 * 
 * @param state The CPU state.
 */
static int32_t syscall_close(isr_cpu_state_t *state);

/**
 * Get sysinfo syscall handler.
 * 
 * Syscall expects the following parameters:
 * 
 * - eax: Syscall number
 * 
 * - ebx: Pointer to info struct
 * 
 * Syscall returns 0 on success or -1 on error.
 * 
 * @param state The CPU state.
 * @return The number of bytes written or -1 on error.
 */
static int32_t syscall_get_osinfo(isr_cpu_state_t *state);

/**
 * Get memory info syscall handler.
 * 
 * Syscall expects the following parameters:
 * 
 * - eax: Syscall number
 * 
 * - ebx: Pointer to info struct
 * 
 * Syscall returns 0 on success or -1 on error.
 * 
 * @param state The CPU state.
 * @return The number of bytes written or -1 on error.
 */
static int32_t syscall_get_meminfo(isr_cpu_state_t *state);

/**
 * Get terminal info syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to terminfo struct
 *
 * Syscall returns 0 on success or -1 on error.
 *
 * @param state The CPU state.
 * @return 0 on success or -1 on error.
 */
static int32_t syscall_get_terminfo(isr_cpu_state_t *state);

/**
 * Allocate/increase heap syscall handler.
 * 
 * Syscall expects the following parameters:
 * 
 * - eax: Syscall number
 * 
 * - ebx: Number of pages to increase the heap by
 * 
 * Syscall returns the new heap end address or NULL on error.
 * 
 * @param state The CPU state.
 * @return The number of bytes written or -1 on error.
 */
static void* syscall_alloc_heap(isr_cpu_state_t *state);

/**
 * Exit syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Exit code
 *
 * @param state The CPU state.
 */
static void syscall_exit(isr_cpu_state_t *state);

/**
 * Open directory syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Directory path
 *
 * Syscall returns the directory descriptor or -1 on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_opendir(isr_cpu_state_t *state);

/**
 * Read directory syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Directory descriptor
 *
 * - ecx: Pointer to a user dirent struct to fill
 *
 * Syscall returns 0 on success or -1 when there are no more entries or on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_readdir(isr_cpu_state_t *state);

/**
 * Close directory syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Directory descriptor
 *
 * Syscall returns 0 on success or -1 on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_closedir(isr_cpu_state_t *state);

/**
 * List volumes syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Index of the volume to query
 *
 * - ecx: Pointer to a user volinfo struct to fill
 *
 * Syscall returns 0 on success or -1 when the index is out of range or on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_lsvol(isr_cpu_state_t *state);

/**
 * Power off syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * On success the machine powers off and the syscall never returns. It only
 * returns -1 when the power off could not be performed.
 *
 * @param state The CPU state.
 */
static int32_t syscall_poweroff(isr_cpu_state_t *state);

/**
 * List devices syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Index of the device to query
 *
 * - ecx: Pointer to a user devinfo struct to fill
 *
 * Syscall returns 0 on success or -1 when the index is out of range or on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_lsdev(isr_cpu_state_t *state);

/**
 * List mount points syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Index of the mount point to query
 *
 * - ecx: Pointer to a user mntinfo struct to fill
 *
 * Syscall returns 0 on success or -1 when the index is out of range or on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_lsmnt(isr_cpu_state_t *state);

/**
 * Mount syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Drive letter to mount to
 *
 * - ecx: Pointer to the short id of the volume to mount
 *
 * Syscall returns 0 on success, -1 if the volume was not found, -2 if the
 * drive is already in use, -3 if the mount failed, or -4 if the volume is
 * already mounted to another drive.
 *
 * @param state The CPU state.
 */
static int32_t syscall_mount(isr_cpu_state_t *state);

/**
 * Unmount syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Drive letter to unmount
 *
 * Syscall returns 0 on success, -1 if the drive is not mounted, -2 if the
 * unmount failed, or -3 if the drive is locked against unmounting.
 *
 * @param state The CPU state.
 */
static int32_t syscall_unmount(isr_cpu_state_t *state);

/**
 * Kernel message log syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Index of the message to query
 *
 * - ecx: Pointer to a user kmsg_entry struct to fill
 *
 * Syscall returns 0 on success or -1 when the index is out of range or on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_dmesg(isr_cpu_state_t *state);

/**
 * Uptime syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * Syscall returns the system uptime in seconds.
 *
 * @param state The CPU state.
 */
static int32_t syscall_uptime(isr_cpu_state_t *state);

/**
 * Memory map syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Index of the memory region to query
 *
 * - ecx: Pointer to a user memregion struct to fill
 *
 * Syscall returns 0 on success or -1 when the index is out of range or on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_memmap(isr_cpu_state_t *state);

/**
 * Get kernel heap info syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to a meminfo struct to fill
 *
 * Syscall returns 0 on success or -1 on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_get_kheapinfo(isr_cpu_state_t *state);

/**
 * Spawn syscall handler.
 *
 * Creates a new child process from an executable and makes it ready to run.
 * The caller continues; the child's outcome is collected with wait.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Path to the executable (user pointer)
 *
 * - ecx: NULL terminated argument vector (user pointer, argv[0] is the path)
 *
 * - edx: NULL terminated environment vector (user pointer, entries of the form
 *        NAME=VALUE), or NULL for an empty environment
 *
 * - esi: Flags. SPAWN_FOREGROUND makes the child the terminal's foreground
 *        process in the same step, which only the current foreground process
 *        may ask for.
 *
 * Syscall returns the PID of the child or -1 if it could not be created or
 * the caller may not hand over the foreground.
 *
 * @param state The CPU state.
 */
static int32_t syscall_spawn(isr_cpu_state_t *state);

/**
 * Set foreground syscall handler.
 *
 * Hands the terminal's foreground to a process. Only the current foreground
 * process may do so (or anyone, while no foreground process exists), and
 * only to itself or one of its children.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: PID of the new foreground process, or 0 for the caller itself
 *
 * Syscall returns 0 on success or -1 if the caller may not set the
 * foreground or the PID is not the caller or a live child of it.
 *
 * @param state The CPU state.
 */
static int32_t syscall_set_foreground(isr_cpu_state_t *state);

/**
 * Get PID syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * Syscall returns the PID of the calling process.
 *
 * @param state The CPU state.
 */
static int32_t syscall_getpid(isr_cpu_state_t *state);

/**
 * Kill syscall handler.
 *
 * Terminates a process. Any process may be killed except init; the parent
 * sees a status of 137 (128 + SIGKILL by the Unix convention).
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: PID of the process to terminate
 *
 * Syscall returns 0 on success or -1 if there is no such process, it has
 * already exited or it is init.
 *
 * @param state The CPU state.
 */
static int32_t syscall_kill(isr_cpu_state_t *state);

/**
 * List processes syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Index of the process to query, in process table order
 *
 * - ecx: Pointer to a user procinfo struct to fill
 *
 * Syscall returns 0 on success or -1 when the index is out of range or on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_lsproc(isr_cpu_state_t *state);

/**
 * Wait syscall handler.
 *
 * Collects the outcome of a child that has exited and destroys it. Blocks
 * until such a child exists unless told not to.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: PID of the child to wait for, or -1 for any child
 *
 * - ecx: Pointer to a user int32_t that receives the status, or NULL. The
 *        status is the exit code masked to a byte or, for a child terminated
 *        by a CPU exception, 128 + the exception number.
 *
 * - edx: Options; WAIT_NOHANG returns instead of blocking
 *
 * Syscall returns the PID of the collected child, 0 if WAIT_NOHANG was given
 * and no child has exited yet, or -1 if the caller has no such child.
 *
 * @param state The CPU state.
 */
static int32_t syscall_wait(isr_cpu_state_t *state);

static char** syscall_copy_vector(char** user_vector, int* count);
static void syscall_free_vector(char** vector, int count);
static int32_t syscall_resolve_path(const char* user_path, char* resolved);

/**
 * Change directory syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Path of the directory to change to, absolute or relative to the
 *        current working directory
 *
 * Syscall returns 0 on success or -1 when the path is malformed, does not
 * exist or is no directory.
 *
 * @param state The CPU state.
 */
static int32_t syscall_chdir(isr_cpu_state_t *state);

/**
 * Get working directory syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to a user buffer to fill
 *
 * - ecx: Size of that buffer in bytes
 *
 * Syscall returns 0 on success or -1 when the buffer is NULL or too small for
 * the working directory including its terminating NUL.
 *
 * @param state The CPU state.
 */
static int32_t syscall_getcwd(isr_cpu_state_t *state);

/**
 * Stat syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Path of the file or directory, absolute or relative to the current
 *        working directory
 *
 * - ecx: Pointer to a user fileinfo struct to fill
 *
 * Syscall returns 0 on success or -1 when the path is malformed or does not
 * exist.
 *
 * @param state The CPU state.
 */
static int32_t syscall_stat(isr_cpu_state_t *state);

/**
 * Raw device read syscall handler.
 *
 * Reads from a storage device past any file system on it, which is what
 * writing a partition table or a boot sector has to be able to do.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to the short id of the device
 *
 * - ecx: Offset into the device in bytes
 *
 * - edx: Number of bytes to read
 *
 * - esi: Pointer to the buffer to read into
 *
 * Syscall returns the number of bytes read or -1 when the device does not
 * exist or is no storage device.
 *
 * @param state The CPU state.
 */
static int32_t syscall_devread(isr_cpu_state_t *state);

/**
 * Raw device write syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to the short id of the device
 *
 * - ecx: Offset into the device in bytes
 *
 * - edx: Number of bytes to write
 *
 * - esi: Pointer to the buffer to write from
 *
 * Syscall returns the number of bytes written or -1 when the device does not
 * exist or is no storage device.
 *
 * @param state The CPU state.
 */
static int32_t syscall_devwrite(isr_cpu_state_t *state);

/**
 * Volume rescan syscall handler.
 *
 * Discards the volumes of a device and scans it again, which is how a
 * partition table written through syscall_devwrite reaches the volume manager.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to the short id of the device
 *
 * Syscall returns the number of volumes found, -1 when the device does not
 * exist or is no storage device, or -2 while one of its volumes is mounted.
 *
 * @param state The CPU state.
 */
static int32_t syscall_rescan(isr_cpu_state_t *state);

/**
 * File system usage syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Drive letter of the mount point
 *
 * - ecx: Pointer to a user fsinfo struct to fill
 *
 * Syscall returns 0 on success or -1 when the drive is not mounted or the file
 * system cannot report its usage.
 *
 * @param state The CPU state.
 */
static int32_t syscall_get_fsinfo(isr_cpu_state_t *state);

/**
 * Storage device info syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to the short id of the device
 *
 * - ecx: Pointer to a user storageinfo struct to fill
 *
 * Syscall returns 0 on success or -1 when the device does not exist or is no
 * storage device.
 *
 * @param state The CPU state.
 */
static int32_t syscall_get_storageinfo(isr_cpu_state_t *state);

/**
 * Raw volume read syscall handler.
 *
 * Reads from a volume past any file system on it, with the offset counted from
 * the start of the volume rather than of the device it sits on. A read that
 * would reach past the volume is cut short at its end.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to the short id of the volume
 *
 * - ecx: Offset into the volume in bytes
 *
 * - edx: Number of bytes to read
 *
 * - esi: Pointer to the buffer to read into
 *
 * Syscall returns the number of bytes read or -1 when the volume does not
 * exist.
 *
 * @param state The CPU state.
 */
static int32_t syscall_volread(isr_cpu_state_t *state);

/**
 * Raw volume write syscall handler.
 *
 * Writes to a volume past any file system on it. A write that would reach past
 * the volume is cut short at its end, so what is written cannot land on a
 * neighbouring partition or the partition table.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Pointer to the short id of the volume
 *
 * - ecx: Offset into the volume in bytes
 *
 * - edx: Number of bytes to write
 *
 * - esi: Pointer to the buffer to write from
 *
 * Syscall returns the number of bytes written or -1 when the volume does not
 * exist.
 *
 * @param state The CPU state.
 */
static int32_t syscall_volwrite(isr_cpu_state_t *state);

/**
 * Unlink syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Path of the file to delete
 *
 * Syscall returns 0 on success or -1 on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_unlink(isr_cpu_state_t *state);

/**
 * Remove directory syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Path of the directory to delete
 *
 * Syscall returns 0 on success or -1 when the path is no directory, the directory is not empty or
 * on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_rmdir(isr_cpu_state_t *state);

/**
 * Make directory syscall handler.
 *
 * Syscall expects the following parameters:
 *
 * - eax: Syscall number
 *
 * - ebx: Path of the directory to create
 *
 * - ecx: Permissions of the new directory
 *
 * Syscall returns 0 on success or -1 when the path already exists or on error.
 *
 * @param state The CPU state.
 */
static int32_t syscall_mkdir(isr_cpu_state_t *state);

void syscall_init() {
    isr_register_listener(SYSCALL_INTERRUPT, syscall_handler);
}

static void syscall_handler(isr_cpu_state_t *state) {
    uint32_t syscall = state->eax;

    switch(syscall) {
        case SYSCALL_READ: {
            state->eax = syscall_read(state);
            break;
        }
        case SYSCALL_WRITE: {
            state->eax = syscall_write(state);
            break;
        }
        case SYSCALL_OPEN: {
            state->eax = syscall_open(state);
            break;
        }
        case SYSCALL_CLOSE: {
            state->eax = syscall_close(state);
            break;
        }
        case SYSCALL_GET_OSINFO: {
            state->eax = syscall_get_osinfo(state);
            break;
        }
        case SYSCALL_GET_MEMINFO: {
            state->eax = syscall_get_meminfo(state);
            break;
        }
        case SYSCALL_GET_TERMINFO: {
            state->eax = syscall_get_terminfo(state);
            break;
        }
        case SYSCALL_ALLOC_HEAP: {
            state->eax = syscall_alloc_heap(state);
            break;
        }
        case SYSCALL_EXIT: {
            syscall_exit(state);
            break;
        }
        case SYSCALL_OPENDIR: {
            state->eax = syscall_opendir(state);
            break;
        }
        case SYSCALL_READDIR: {
            state->eax = syscall_readdir(state);
            break;
        }
        case SYSCALL_CLOSEDIR: {
            state->eax = syscall_closedir(state);
            break;
        }
        case SYSCALL_LSVOL: {
            state->eax = syscall_lsvol(state);
            break;
        }
        case SYSCALL_POWEROFF: {
            state->eax = syscall_poweroff(state);
            break;
        }
        case SYSCALL_LSDEV: {
            state->eax = syscall_lsdev(state);
            break;
        }
        case SYSCALL_LSMNT: {
            state->eax = syscall_lsmnt(state);
            break;
        }
        case SYSCALL_MOUNT: {
            state->eax = syscall_mount(state);
            break;
        }
        case SYSCALL_UNMOUNT: {
            state->eax = syscall_unmount(state);
            break;
        }
        case SYSCALL_DMESG: {
            state->eax = syscall_dmesg(state);
            break;
        }
        case SYSCALL_UPTIME: {
            state->eax = syscall_uptime(state);
            break;
        }
        case SYSCALL_MEMMAP: {
            state->eax = syscall_memmap(state);
            break;
        }
        case SYSCALL_GET_KHEAPINFO: {
            state->eax = syscall_get_kheapinfo(state);
            break;
        }
        case SYSCALL_SPAWN: {
            state->eax = syscall_spawn(state);
            break;
        }
        case SYSCALL_UNLINK: {
            state->eax = syscall_unlink(state);
            break;
        }
        case SYSCALL_RMDIR: {
            state->eax = syscall_rmdir(state);
            break;
        }
        case SYSCALL_MKDIR: {
            state->eax = syscall_mkdir(state);
            break;
        }
        case SYSCALL_CHDIR: {
            state->eax = syscall_chdir(state);
            break;
        }
        case SYSCALL_GETCWD: {
            state->eax = syscall_getcwd(state);
            break;
        }
        case SYSCALL_STAT: {
            state->eax = syscall_stat(state);
            break;
        }
        case SYSCALL_DEVREAD: {
            state->eax = syscall_devread(state);
            break;
        }
        case SYSCALL_DEVWRITE: {
            state->eax = syscall_devwrite(state);
            break;
        }
        case SYSCALL_RESCAN: {
            state->eax = syscall_rescan(state);
            break;
        }
        case SYSCALL_GET_FSINFO: {
            state->eax = syscall_get_fsinfo(state);
            break;
        }
        case SYSCALL_GET_STORAGEINFO: {
            state->eax = syscall_get_storageinfo(state);
            break;
        }
        case SYSCALL_VOLREAD: {
            state->eax = syscall_volread(state);
            break;
        }
        case SYSCALL_VOLWRITE: {
            state->eax = syscall_volwrite(state);
            break;
        }
        case SYSCALL_WAIT: {
            state->eax = syscall_wait(state);
            break;
        }
        case SYSCALL_SET_FOREGROUND: {
            state->eax = syscall_set_foreground(state);
            break;
        }
        case SYSCALL_GETPID: {
            state->eax = syscall_getpid(state);
            break;
        }
        case SYSCALL_LSPROC: {
            state->eax = syscall_lsproc(state);
            break;
        }
        case SYSCALL_KILL: {
            state->eax = syscall_kill(state);
            break;
        }
        default: {
            state->eax = -1;
            break;
        }
    }
}

static int32_t syscall_read(isr_cpu_state_t *state) {
    int32_t fd = state->ebx;
    uint8_t* buffer = (uint8_t*) state->ecx;
    size_t size = state->edx;

    if(fd < 0 || fd >= PROCESS_MAX_FILE_DESCRIPTORS) {
        return -1;
    }

    process_t* current_process = process_get_current();

    // Read from stdin: sleeps until input arrives, foreground process only.
    if(current_process && current_process->in && fd == 0) {
        return stream_read(current_process->in, (char*) buffer, size);
    }

    // Read from file
    if(current_process && current_process->files[fd]) {
        return file_read(current_process->files[fd], buffer, size);
    }

    return -1;
}

static int32_t syscall_write(isr_cpu_state_t *state) {
    int32_t fd = state->ebx;
    const uint8_t* buffer = (const uint8_t*) state->ecx;
    size_t size = state->edx;

    if(fd < 0 || fd >= PROCESS_MAX_FILE_DESCRIPTORS) {
        return -1;
    }

    process_t* current_process = process_get_current();

    // Write to stdout
    if(current_process && current_process->out && fd == 1) {
        return stream_write(current_process->out, (const char*) buffer, size);
    }

    // Write to stderr
    if(current_process && current_process->err && fd == 2) {
        return stream_write(current_process->err, (const char*) buffer, size);
    }

    // Write to file
    if(current_process && current_process->files[fd]) {
        return file_write(current_process->files[fd], buffer, size);
    }

    return -1;
}

static int32_t syscall_open(isr_cpu_state_t *state) {
    const char* name = (const char*) state->ebx;
    int32_t flags = state->ecx;
    int32_t mode = state->edx;

    process_t* current_process = process_get_current();

    if(current_process) {
        int32_t fd = -1;

        // Find first free file descriptor, skip over stdin/stdout/stderr
        for(int32_t index = 3; index < PROCESS_MAX_FILE_DESCRIPTORS; index++) {
            if(current_process->files[index] == NULL) {
                fd = index;
                break;
            }
        }

        if(fd == -1) {
            return -1;
        }

        char resolved[PATH_MAX];

        if(!name || path_resolve(current_process->cwd, name, resolved) != 0) {
            return -1;
        }

        file_descriptor_t* file_descriptor = file_open(resolved, flags, (uint32_t) mode);

        if(file_descriptor) {
            current_process->files[fd] = file_descriptor;
            return fd;
        }
    }

    return -1;
}

static int32_t syscall_unlink(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;
    char resolved[PATH_MAX];

    if(syscall_resolve_path(path, resolved) != 0) {
        return -1;
    }

    return file_unlink(resolved);
}

static int32_t syscall_rmdir(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;
    char resolved[PATH_MAX];

    if(syscall_resolve_path(path, resolved) != 0) {
        return -1;
    }

    return file_rmdir(resolved);
}

static int32_t syscall_mkdir(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;
    int32_t mode = state->ecx;
    char resolved[PATH_MAX];

    if(syscall_resolve_path(path, resolved) != 0) {
        return -1;
    }

    return file_mkdir(resolved, (uint32_t) mode);
}

static int32_t syscall_close(isr_cpu_state_t *state) {
    int32_t fd = state->ebx;

    if(fd < 0 || fd >= PROCESS_MAX_FILE_DESCRIPTORS) {
        return -1;
    }

    process_t* current_process = process_get_current();

    if(current_process) {
        // Skip stdin/stdout/stderr
        if(fd < 3) {
            return -1;
        }

        if(current_process->files[fd] != NULL) {
            file_close(current_process->files[fd]);
            current_process->files[fd] = NULL;
            return 0;
        }
    }

    return -1;
}

static int32_t syscall_get_osinfo(isr_cpu_state_t *state) {
    struct osinfo* info = (struct osinfo*) state->ebx;

    strncpy(info->name, __KERNEL_NAME__, 16);
    info->name[15] = '\0';

    strncpy(info->version, __KERNEL_VERSION__, 32);
    info->version[31] = '\0';

    strncpy(info->arch, __KERNEL_ARCH__, 16);
    info->version[15] = '\0';

    strncpy(info->platform, __KERNEL_PLATFORM__, 16);
    info->version[15] = '\0';

    return 0;
}

static int32_t syscall_get_meminfo(isr_cpu_state_t *state) {
    struct meminfo* info = (struct meminfo*) state->ebx;

    info->total = pmm_get_total_memory_size();
    info->free = pmm_get_available_memory_size();

    return 0;
}

static int32_t syscall_get_terminfo(isr_cpu_state_t *state) {
    struct terminfo* info = (struct terminfo*) state->ebx;

    process_t* current_process = process_get_current();

    if(!current_process || !current_process->out) {
        return -1;
    }

    // The process' output stream is backed by a TTY, which knows its dimensions.
    tty_t* tty = (tty_t*) current_process->out->data;

    if(!tty) {
        return -1;
    }

    info->rows = tty->rows;
    info->cols = tty->columns;

    return 0;
}

static void* syscall_alloc_heap(isr_cpu_state_t *state) {
    uint32_t n_pages = state->ebx;

    process_t* current_process = process_get_current();

    if(current_process) {
        void* heap_start = current_process->heap_base;
        void* current_heap_end = current_process->heap_limit;

        if(n_pages == 0) {
            return current_process->heap_limit;
        }

        // If the heap is not allocated, allocate it
        if(heap_start == NULL) {
            heap_start = vmm_map_memory(NULL, n_pages * PAGE_SIZE, NULL, false, true);

            if(!heap_start) {
                return NULL;
            }

            current_process->heap_base = heap_start;
            current_process->heap_limit = (void*) ((uint32_t) heap_start + (n_pages * PAGE_SIZE) - 1);
        } else {
            void* block_begin = vmm_map_memory((void*) ((uint32_t) current_heap_end + 1), n_pages * PAGE_SIZE, NULL, false, true);

            if(!block_begin) {
                return NULL;
            }

            current_process->heap_limit = (void*) ((uint32_t) block_begin + (n_pages * PAGE_SIZE) - 1);
        }

        return current_process->heap_limit;
    }
}

void syscall_exit(isr_cpu_state_t *state) {
    int32_t exit_code = state->ebx;

    process_t* current_process = process_get_current();

    if(current_process) {
        process_exit(exit_code, -1);
    }

    while(1);
}

static int32_t syscall_opendir(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;
    char resolved[PATH_MAX];

    if(syscall_resolve_path(path, resolved) != 0) {
        return -1;
    }

    return dir_open(resolved);
}

static int32_t syscall_readdir(isr_cpu_state_t *state) {
    int32_t dd = state->ebx;
    struct dirent* user_entry = (struct dirent*) state->ecx;

    if(!user_entry) {
        return -1;
    }

    // dir_read returns a kmalloc'd entry; copy its fields into the caller's
    // buffer and free it instead of handing a kernel pointer to userland.
    const dir_dirent_t* dirent = dir_read(dd);

    if(!dirent) {
        return -1;
    }

    strncpy(user_entry->name, dirent->name, sizeof(user_entry->name));
    user_entry->name[sizeof(user_entry->name) - 1] = '\0';
    user_entry->inode = dirent->inode;

    kfree((void*) dirent);

    return 0;
}

static int32_t syscall_closedir(isr_cpu_state_t *state) {
    int32_t dd = state->ebx;

    return dir_close(dd);
}

static int32_t syscall_lsvol(isr_cpu_state_t *state) {
    uint32_t index = state->ebx;
    struct volinfo* info = (struct volinfo*) state->ecx;

    if(!info) {
        return -1;
    }

    // Index-based iterator over the global volume list, so userland can walk it
    // by calling with 0, 1, 2, ... until -1 signals the end.
    linked_list_node_t* node = linked_list_get((linked_list_t*) volume_get_all(), index);

    if(!node) {
        return -1;
    }

    volume_t* volume = (volume_t*) node->data;

    strncpy(info->name, volume->name, sizeof(info->name));
    info->name[sizeof(info->name) - 1] = '\0';

    strncpy(info->id, volume->id, sizeof(info->id));
    info->id[sizeof(info->id) - 1] = '\0';

    strncpy(info->device_id, volume->device->id, sizeof(info->device_id));
    info->device_id[sizeof(info->device_id) - 1] = '\0';

    info->size = volume->size;

    /*
     * Probed on every call rather than remembered: a volume that is formatted
     * while the system runs carries a different file system afterwards.
     */
    mnt_probe_volume(volume, info->fs_type, info->label);

    return 0;
}

static int32_t syscall_poweroff(isr_cpu_state_t *state) {
    (void) state;

    // On success the machine powers off here and never returns; a return value
    // means the power off failed and is reported back to userland.
    return acpi_poweroff();
}

struct lsdev_iterator {
    uint32_t target_index;
    uint32_t current_index;
    generic_tree_node_t* result;
};

static void syscall_lsdev_callback(generic_tree_node_t* node, void* userdata) {
    struct lsdev_iterator* iterator = (struct lsdev_iterator*) userdata;

    if(iterator->current_index == iterator->target_index) {
        iterator->result = node;
    }

    iterator->current_index++;
}

static int32_t syscall_lsdev(isr_cpu_state_t *state) {
    uint32_t index = state->ebx;
    struct devinfo* info = (struct devinfo*) state->ecx;

    if(!info) {
        return -1;
    }

    // The device manager stores devices in a tree without index access, so walk
    // it in pre-order and pick the node at the requested index. Userland calls
    // this with 0, 1, 2, ... until -1 signals the end.
    struct lsdev_iterator iterator = { .target_index = index, .current_index = 0, .result = NULL };

    generic_tree_foreach((generic_tree_t*) device_get_all(), syscall_lsdev_callback, &iterator);

    if(!iterator.result) {
        return -1;
    }

    device_t* device = (device_t*) iterator.result->data;

    strncpy(info->name, device->name, sizeof(info->name));
    info->name[sizeof(info->name) - 1] = '\0';

    strncpy(info->id, device->id, sizeof(info->id));
    info->id[sizeof(info->id) - 1] = '\0';

    info->type = device->type;
    info->bus_type = device->bus.type;

    /*
     * The pre-order walk hands out a flat list, so the position in the tree
     * would be lost. Walking back up to the root restores it and lets userland
     * indent the listing.
     */
    uint8_t depth = 0;

    for(generic_tree_node_t* node = iterator.result->parent; node != NULL; node = node->parent) {
        depth++;
    }

    info->depth = depth;

    /*
     * Nodes are appended to the sibling chain, so the last child is the one
     * without a successor.
     */
    uint32_t last_child_mask = 0;
    uint8_t level = depth;

    for(generic_tree_node_t* node = iterator.result; node->parent != NULL; node = node->parent) {
        if(node->next == NULL && level < 32) {
            last_child_mask |= 1u << level;
        }

        level--;
    }

    info->last_child_mask = last_child_mask;

    return 0;
}

static int32_t syscall_lsmnt(isr_cpu_state_t *state) {
    uint32_t index = state->ebx;
    struct mntinfo* info = (struct mntinfo*) state->ecx;

    if(!info) {
        return -1;
    }

    // Mount points are keyed by drive letter (A-Z) and sparse, so walk the range
    // and pick the index-th occupied slot. Userland calls this with 0, 1, 2, ...
    // until -1 signals the end.
    uint32_t current_index = 0;

    for(char drive = DRIVE_A; drive <= DRIVE_Z; drive++) {
        const vfs_filesystem_t* filesystem = mnt_get_drive(drive);

        if(filesystem != NULL) {
            if(current_index == index) {
                info->drive = drive;

                strncpy(info->volume_id, filesystem->volume->id, sizeof(info->volume_id));
                info->volume_id[sizeof(info->volume_id) - 1] = '\0';

                strncpy(info->fs_type, filesystem->type, sizeof(info->fs_type));
                info->fs_type[sizeof(info->fs_type) - 1] = '\0';

                return 0;
            }

            current_index++;
        }
    }

    return -1;
}

static int32_t syscall_mount(isr_cpu_state_t *state) {
    char drive = (char) state->ebx;
    const char* id = (const char*) state->ecx;

    if(!id) {
        return -1;
    }

    const volume_t* volume = volume_find_by_id(id);

    if(!volume) {
        return -1;
    }

    if(mnt_get_drive(drive) != NULL) {
        return -2;
    }

    if(mnt_get_volume_drive(volume) != 0) {
        return -4;
    }

    if(mnt_volume_mount(drive, (volume_t*) volume) != 0) {
        return -3;
    }

    return 0;
}

static int32_t syscall_unmount(isr_cpu_state_t *state) {
    char drive = (char) state->ebx;

    if(mnt_get_drive(drive) == NULL) {
        return -1;
    }

    if(mnt_drive_is_locked(drive)) {
        return -3;
    }

    if(mnt_volume_unmount(drive) != 0) {
        return -2;
    }

    return 0;
}

static int32_t syscall_dmesg(isr_cpu_state_t *state) {
    uint32_t index = state->ebx;
    struct kmsg_entry* entry = (struct kmsg_entry*) state->ecx;

    if(!entry) {
        return -1;
    }

    // Index-based iterator over the kernel message log, so userland can walk it
    // by calling with 0, 1, 2, ... until -1 signals the end.
    linked_list_node_t* node = linked_list_get((linked_list_t*) kmessage_get_messages(), index);

    if(!node) {
        return -1;
    }

    kmessage_message_t* message = (kmessage_message_t*) node->data;

    strncpy(entry->level, message->level, sizeof(entry->level));
    entry->level[sizeof(entry->level) - 1] = '\0';

    strncpy(entry->message, message->message, sizeof(entry->message));
    entry->message[sizeof(entry->message) - 1] = '\0';

    return 0;
}

static int32_t syscall_uptime(isr_cpu_state_t *state) {
    (void) state;

    return (int32_t) timer_get_uptime();
}

static int32_t syscall_memmap(isr_cpu_state_t *state) {
    uint32_t index = state->ebx;
    struct memregion* region = (struct memregion*) state->ecx;

    if(!region) {
        return -1;
    }

    // Index-based iterator over the PMM memory regions, so userland can walk
    // them by calling with 0, 1, 2, ... until -1 signals the end.
    linked_list_node_t* node = linked_list_get((linked_list_t*) pmm_get_memory_regions(), index);

    if(!node) {
        return -1;
    }

    pmm_memory_region_t* memory_region = (pmm_memory_region_t*) node->data;

    region->base = memory_region->base;
    region->length = memory_region->length;
    region->type = memory_region->type;

    return 0;
}

static int32_t syscall_get_kheapinfo(isr_cpu_state_t *state) {
    struct meminfo* info = (struct meminfo*) state->ebx;

    if(!info) {
        return -1;
    }

    info->total = kheap_get_total_memory_size();
    info->free = kheap_get_available_memory_size();

    return 0;
}

/**
 * Copies a NULL terminated vector of strings out of the caller's user address
 * space into kernel memory. On success *count holds the number of entries and
 * the returned array has that many kernel copies; NULL is returned when memory
 * runs out. A NULL vector counts as empty.
 */
static char** syscall_copy_vector(char** user_vector, int* count) {
    int length = 0;

    if(user_vector) {
        while(user_vector[length] != NULL) {
            length++;
        }
    }

    char** kernel_vector = kmalloc((length > 0 ? length : 1) * sizeof(char*));

    if(!kernel_vector) {
        return NULL;
    }

    for(int index = 0; index < length; index++) {
        size_t size = strlen(user_vector[index]) + 1;
        kernel_vector[index] = kmalloc(size);

        if(!kernel_vector[index]) {
            syscall_free_vector(kernel_vector, index);
            return NULL;
        }

        memcpy(kernel_vector[index], user_vector[index], size);
    }

    *count = length;

    return kernel_vector;
}

static void syscall_free_vector(char** vector, int count) {
    for(int index = 0; index < count; index++) {
        kfree(vector[index]);
    }

    kfree(vector);
}

static int32_t syscall_spawn(isr_cpu_state_t *state) {
    const char* user_path = (const char*) state->ebx;
    char** user_argv = (char**) state->ecx;
    char** user_envp = (char**) state->edx;
    uint32_t flags = state->esi;

    process_t* parent = (process_t*) process_get_current();

    if(!parent || !user_path) {
        return -1;
    }

    tty_t* tty = (tty_t*) tty_get_stdterm();

    // Checked up front: once the child exists it cannot be taken back.
    if((flags & SPAWN_FOREGROUND) && !tty_may_set_foreground(tty, parent)) {
        return -1;
    }

    /*
     * Marshal the path, the argument vector and the environment out of the
     * parent's user address space into kernel memory. process_create copies
     * the strings only after switching into the child's address space, where
     * the parent's user pointers are no longer mapped. The kernel stack and
     * heap, on the other hand, are mapped in every address space, so the
     * copies stay valid across the switch.
     */

    char kernel_path[PATH_MAX];

    if(path_resolve(parent->cwd, user_path, kernel_path) != 0) {
        return -1;
    }

    int argc = 0;
    int envc = 0;

    // argv is NULL terminated; argv[0] is conventionally the executable path.
    char** kernel_argv = syscall_copy_vector(user_argv, &argc);

    if(!kernel_argv) {
        return -1;
    }

    char** kernel_envp = syscall_copy_vector(user_envp, &envc);

    if(!kernel_envp) {
        syscall_free_vector(kernel_argv, argc);
        return -1;
    }

    // The child is named after its executable and starts where its parent stands.
    const char* name = kernel_path;

    for(const char* cursor = kernel_path; *cursor != '\0'; cursor++) {
        if(*cursor == '/') {
            name = cursor + 1;
        }
    }

    process_t* child = process_create(name, kernel_path, argc, (const char**) kernel_argv, envc, (const char**) kernel_envp, parent->cwd, parent->out, parent->in, parent->err);

    // process_create has copied path, arguments and environment onto the child's stack.
    syscall_free_vector(kernel_argv, argc);
    syscall_free_vector(kernel_envp, envc);

    if(!child) {
        return -1;
    }

    child->parent = parent;

    /*
     * Done before returning, with interrupts still disabled: the child cannot
     * run and read before it is the foreground process.
     */
    if(flags & SPAWN_FOREGROUND) {
        tty_set_foreground(tty, parent, child->pid);
    }

    return child->pid;
}

static int32_t syscall_getpid(isr_cpu_state_t *state) {
    (void) state;

    const process_t* current = process_get_current();

    return current != NULL ? current->pid : -1;
}

static int32_t syscall_kill(isr_cpu_state_t *state) {
    pid_t pid = (pid_t) state->ebx;

    if(pid <= 1) {
        return -1;
    }

    process_t* process = (process_t*) process_get_by_pid(pid);

    if(process == NULL || process->state == PROCESS_STATE_EXITED) {
        return -1;
    }

    // Does not return if the caller kills itself.
    process_kill(process, 137);

    return 0;
}

static int32_t syscall_lsproc(isr_cpu_state_t *state) {
    uint32_t index = state->ebx;
    procinfo_t* info = (procinfo_t*) state->ecx;

    if(!info) {
        return -1;
    }

    const process_t* process = process_get_by_index(index);

    if(process == NULL) {
        return -1;
    }

    info->pid = process->pid;
    info->parent = process->parent != NULL ? process->parent->pid : 0;

    switch(process->state) {
        case PROCESS_STATE_READY:   info->state = PROC_STATE_READY;   break;
        case PROCESS_STATE_RUNNING: info->state = PROC_STATE_RUNNING; break;
        case PROCESS_STATE_EXITED:  info->state = PROC_STATE_EXITED;  break;
        case PROCESS_STATE_WAITING: info->state = PROC_STATE_WAITING; break;
    }

    strncpy(info->name, process->name, sizeof(info->name));
    info->name[sizeof(info->name) - 1] = '\0';

    return 0;
}

static int32_t syscall_set_foreground(isr_cpu_state_t *state) {
    pid_t pid = (pid_t) state->ebx;

    process_t* caller = (process_t*) process_get_current();

    if(!caller) {
        return -1;
    }

    tty_t* tty = (tty_t*) tty_get_stdterm();

    if(!tty_may_set_foreground(tty, caller)) {
        return -1;
    }

    const process_t* target = caller;

    if(pid != 0 && pid != caller->pid) {
        target = process_find_child(caller, pid);

        if(target == NULL || target->state == PROCESS_STATE_EXITED) {
            return -1;
        }
    }

    tty_set_foreground(tty, caller, target->pid);

    return 0;
}

static int32_t syscall_wait(isr_cpu_state_t *state) {
    pid_t pid = (pid_t) state->ebx;
    int32_t* user_status = (int32_t*) state->ecx;
    uint32_t options = state->edx;

    process_t* parent = (process_t*) process_get_current();

    if(!parent || (pid < 1 && pid != -1)) {
        return -1;
    }

    for(;;) {
        process_t* child = process_find_child(parent, pid);

        if(child == NULL) {
            return -1;
        }

        if(child->state == PROCESS_STATE_EXITED) {
            /*
             * The status is always non-negative: a normal exit code (masked to
             * a byte) or, for a process terminated by a CPU exception, 128 +
             * the exception number. This lets a caller reserve negative values
             * for "could not execute".
             */
            if(user_status != NULL) {
                if(child->exception_code != -1) {
                    *user_status = 128 + child->exception_code;
                } else {
                    *user_status = child->exit_code & 0xFF;
                }
            }

            pid_t child_pid = child->pid;

            process_destroy(child);

            return child_pid;
        }

        if(options & WAIT_NOHANG) {
            return 0;
        }

        if(process_kill_pending()) {
            return -1;
        }

        // Woken by an exiting child; look again, it may have been another one.
        wait_queue_sleep(&parent->child_exited);
    }
}

/**
 * Resolves a path handed in by userland against the working directory of the
 * current process. Fails when there is no current process, the path is NULL
 * or path_resolve refuses it.
 */
static int32_t syscall_resolve_path(const char* user_path, char* resolved) {
    const process_t* current_process = process_get_current();

    if(!current_process || !user_path) {
        return -1;
    }

    return path_resolve(current_process->cwd, user_path, resolved);
}

static int32_t syscall_chdir(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;
    char resolved[PATH_MAX];

    if(syscall_resolve_path(path, resolved) != 0) {
        return -1;
    }

    // Only a directory that can be opened becomes the working directory.
    int32_t dd = dir_open(resolved);

    if(dd < 0) {
        return -1;
    }

    dir_close(dd);

    process_t* current_process = (process_t*) process_get_current();

    strcpy(current_process->cwd, resolved);

    return 0;
}

static int32_t syscall_getcwd(isr_cpu_state_t *state) {
    char* buffer = (char*) state->ebx;
    size_t size = state->ecx;

    const process_t* current_process = process_get_current();

    if(!current_process || !buffer) {
        return -1;
    }

    if(strlen(current_process->cwd) + 1 > size) {
        return -1;
    }

    strcpy(buffer, current_process->cwd);

    return 0;
}

static int32_t syscall_stat(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;
    struct fileinfo* info = (struct fileinfo*) state->ecx;
    char resolved[PATH_MAX];

    if(!info || syscall_resolve_path(path, resolved) != 0) {
        return -1;
    }

    file_stat_t stat;

    if(file_stat(resolved, &stat) != 0) {
        return -1;
    }

    // The VFS type codes are the kernel's own; the ABI has its own set.
    switch(stat.type) {
        case VFS_FILE:
            info->type = FILE_TYPE_FILE;
            break;
        case VFS_DIRECTORY:
            info->type = FILE_TYPE_DIRECTORY;
            break;
        case VFS_SYMLINK:
            info->type = FILE_TYPE_SYMLINK;
            break;
        default:
            return -1;
    }

    info->size = stat.size;
    info->inode = stat.inode;

    strncpy(info->volume_id, stat.volume_id, sizeof(info->volume_id));
    info->volume_id[sizeof(info->volume_id) - 1] = '\0';

    info->permissions = stat.permissions;
    info->uid = stat.uid;
    info->gid = stat.gid;

    return 0;
}

/**
 * Looks a storage device up by the short id userland named it with. Returns
 * NULL for an unknown id or a device that is not backed by a storage driver.
 */
static storage_device_t* syscall_find_storage_device(const char* id) {
    if(!id) {
        return NULL;
    }

    const device_t* device = device_find_by_id(id);

    if(!device || device->type != DEVICE_TYPE_STORAGE || !device->driver.storage) {
        return NULL;
    }

    return (storage_device_t*) device;
}

static int32_t syscall_devread(isr_cpu_state_t *state) {
    const char* id = (const char*) state->ebx;
    size_t offset = state->ecx;
    size_t size = state->edx;
    char* buffer = (char*) state->esi;

    storage_device_t* device = syscall_find_storage_device(id);

    if(!device || !buffer) {
        return -1;
    }

    return (int32_t) device->driver.storage->read(device, offset, size, buffer);
}

static int32_t syscall_devwrite(isr_cpu_state_t *state) {
    const char* id = (const char*) state->ebx;
    size_t offset = state->ecx;
    size_t size = state->edx;
    char* buffer = (char*) state->esi;

    storage_device_t* device = syscall_find_storage_device(id);

    if(!device || !buffer) {
        return -1;
    }

    return (int32_t) device->driver.storage->write(device, offset, size, buffer);
}

static int32_t syscall_rescan(isr_cpu_state_t *state) {
    const char* id = (const char*) state->ebx;

    storage_device_t* device = syscall_find_storage_device(id);

    if(!device) {
        return -1;
    }

    /*
     * The volumes are about to be freed, so none of them may still be mounted:
     * a mounted file system holds the volume it reads through.
     */
    linked_list_foreach(volume_get_all(), node) {
        volume_t* volume = (volume_t*) node->data;

        if(volume->device == device && mnt_get_volume_drive(volume) != 0) {
            return -2;
        }
    }

    volume_unregister_device(device);

    return (int32_t) volume_register_device(device);
}

static int32_t syscall_get_fsinfo(isr_cpu_state_t *state) {
    char drive = (char) state->ebx;
    struct fsinfo* info = (struct fsinfo*) state->ecx;

    if(!info) {
        return -1;
    }

    const vfs_filesystem_t* filesystem = mnt_get_drive(drive);

    if(!filesystem || !filesystem->operations->usage) {
        return -1;
    }

    vfs_usage_t usage;

    if(filesystem->operations->usage((vfs_filesystem_t*) filesystem, &usage) != 0) {
        return -1;
    }

    info->total = usage.total;
    info->free = usage.free;

    return 0;
}

static int32_t syscall_get_storageinfo(isr_cpu_state_t *state) {
    const char* id = (const char*) state->ebx;
    struct storageinfo* info = (struct storageinfo*) state->ecx;

    storage_device_t* device = syscall_find_storage_device(id);

    if(!device || !info) {
        return -1;
    }

    info->size = device->driver.storage->total_size(device);
    info->sector_size = device->driver.storage->sector_size(device);

    return 0;
}

static int32_t syscall_volread(isr_cpu_state_t *state) {
    const char* id = (const char*) state->ebx;
    size_t offset = state->ecx;
    size_t size = state->edx;
    char* buffer = (char*) state->esi;

    if(!id || !buffer) {
        return -1;
    }

    const volume_t* volume = volume_find_by_id(id);

    if(!volume) {
        return -1;
    }

    return (int32_t) volume->operations->read((volume_t*) volume, offset, size, buffer);
}

static int32_t syscall_volwrite(isr_cpu_state_t *state) {
    const char* id = (const char*) state->ebx;
    size_t offset = state->ecx;
    size_t size = state->edx;
    char* buffer = (char*) state->esi;

    if(!id || !buffer) {
        return -1;
    }

    const volume_t* volume = volume_find_by_id(id);

    if(!volume) {
        return -1;
    }

    return (int32_t) volume->operations->write((volume_t*) volume, offset, size, buffer);
}
