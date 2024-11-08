#include <system/syscall.h>
#include <arch/i386/isr.h>
#include <io/stream.h>
#include <io/file.h>
#include <system/process.h>
#include <kernel.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <util/string.h>

struct osinfo {
    char name[16];
    char arch[16];
    char version[32];
    char platform[16];
};

struct meminfo {
    size_t total;
    size_t free;
};

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
        case SYSCALL_ALLOC_HEAP: {
            state->eax = syscall_alloc_heap(state);
            break;
        }
        case SYSCALL_EXIT: {
            syscall_exit(state);
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

        kfree(buffer);
        return size;
    }

    // Write to stderr
    if(current_process && current_process->err && fd == 2) {
        char* message = kmalloc(size + 1);
        memcpy(message, buffer, size);
        message[size] = '\0';

        stream_puts(current_process->err, message);

        kfree(buffer);
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
        // Creating files is not supported yet
        if(flags & FILE_CREAT) {
            return -1;
        }

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

        file_descriptor_t* file_descriptor = file_open(name, flags);

        if(file_descriptor) {
            current_process->files[fd] = file_descriptor;
            return fd;
        }
    }

    return -1;
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
        process_terminate(current_process);
    }

    while(1);
}
