#include <system/syscall.h>
#include <arch/i386/isr.h>
#include <io/stream.h>
#include <io/file.h>
#include <io/dir.h>
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
 * Creates a new child process from an executable and runs it to completion,
 * blocking the caller until the child exits. The caller is resumed by
 * process_terminate once the child exits.
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
 * On success this syscall returns the child's exit code (delivered when the
 * child exits); it returns -1 if the child could not be created.
 *
 * @param state The CPU state.
 */
static int32_t syscall_spawn(isr_cpu_state_t *state);

static char** syscall_copy_vector(char** user_vector, int* count);
static void syscall_free_vector(char** vector, int count);

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

    // Read from stdin
    if(current_process && current_process->in && fd == 0) {
        char ch;
        for(size_t i = 0; i < size; i++) {
            /*
             * It is important to read character wise using getchar, because gets or
             * similar functions will read until it encounters a newline character
             * and hence will block the whole system because interrupts are disabled
             * during syscalls. It will result in a deadlock because interrupts are
             * required to handle keyboard input. Because getchar is implemented using
             * raw instead of canonical mode, it will not block on missing input.
             */

            if((ch = stream_getchar(current_process->in)) <= 0) {
                return i;
            }

            buffer[i] = ch;
        }

        return size;
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
        char* message = kmalloc(size + 1);
        memcpy(message, buffer, size);
        message[size] = '\0';

        stream_puts(current_process->out, message);

        kfree(message);
        return size;
    }

    // Write to stderr
    if(current_process && current_process->err && fd == 2) {
        char* message = kmalloc(size + 1);
        memcpy(message, buffer, size);
        message[size] = '\0';

        stream_puts(current_process->err, message);

        kfree(message);
        return size;
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

        file_descriptor_t* file_descriptor = file_open((char*) name, flags, (uint32_t) mode);

        if(file_descriptor) {
            current_process->files[fd] = file_descriptor;
            return fd;
        }
    }

    return -1;
}

static int32_t syscall_unlink(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;

    if(!path) {
        return -1;
    }

    return file_unlink((char*) path);
}

static int32_t syscall_rmdir(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;

    if(!path) {
        return -1;
    }

    return file_rmdir((char*) path);
}

static int32_t syscall_mkdir(isr_cpu_state_t *state) {
    const char* path = (const char*) state->ebx;
    int32_t mode = state->ecx;

    if(!path) {
        return -1;
    }

    return file_mkdir((char*) path, (uint32_t) mode);
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
        current_process->exit_code = exit_code;
        current_process->exception_code = -1;
        process_terminate(current_process);
    }

    while(1);
}

static int32_t syscall_opendir(isr_cpu_state_t *state) {
    char* path = (char*) state->ebx;

    return dir_open(path);
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

    info->size = volume->size;

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

    process_t* parent = process_get_current();

    if(!parent || !user_path) {
        return -1;
    }

    /*
     * Marshal the path, the argument vector and the environment out of the
     * parent's user address space into kernel memory. process_create copies
     * the strings only after switching into the child's address space, where
     * the parent's user pointers are no longer mapped. The kernel heap, on the
     * other hand, is mapped in every address space, so the copies stay valid
     * across the switch.
     */

    size_t path_length = strlen(user_path) + 1;
    char* kernel_path = kmalloc(path_length);

    if(!kernel_path) {
        return -1;
    }

    memcpy(kernel_path, user_path, path_length);

    int argc = 0;
    int envc = 0;

    // argv is NULL terminated; argv[0] is conventionally the executable path.
    char** kernel_argv = syscall_copy_vector(user_argv, &argc);

    if(!kernel_argv) {
        kfree(kernel_path);
        return -1;
    }

    char** kernel_envp = syscall_copy_vector(user_envp, &envc);

    if(!kernel_envp) {
        syscall_free_vector(kernel_argv, argc);
        kfree(kernel_path);
        return -1;
    }

    // Preserve the parent's context so it can be resumed once the child exits.
    parent->saved_state = *state;

    process_t* child = process_create("child", kernel_path, argc, (const char**) kernel_argv, envc, (const char**) kernel_envp, parent->out, parent->in, parent->err);

    // process_create has copied path, arguments and environment onto the child's stack.
    syscall_free_vector(kernel_argv, argc);
    syscall_free_vector(kernel_envp, envc);
    kfree(kernel_path);

    if(!child) {
        return -1;
    }

    child->parent = parent;
    parent->state = PROCESS_STATE_WAITING;

    process_run(child);

    // process_run does not return; the parent is resumed via process_terminate.
    return 0;
}
