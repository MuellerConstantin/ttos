#include <system/process.h>
#include <arch/i386/isr.h>
#include <io/file.h>
#include <system/kpanic.h>
#include <system/elf.h>
#include <memory/kheap.h>
#include <util/string.h>
#include <util/linked_list.h>

static process_t* current_process = NULL;

/* Every process that exists, from process_create until process_destroy. */
static linked_list_t* process_list = NULL;

static pid_t process_next_pid();

static void process_register(process_t* process);
static void process_unregister(process_t* process);
static bool process_compare_pid(void* node_data, void* compare_data);

static size_t process_vector_size(int count, const char** vector);

process_t* process_create(const char* name, const char* path, int argc, const char** argv, int envc, const char** envp, const char* cwd, stream_t* out, stream_t* in, stream_t* err) {
    /*
     * Checked before anything is allocated: the strings of both vectors, their
     * pointer arrays with a NULL terminator each, argc and the alignment slack
     * all land on the same page as the stack itself.
     */
    size_t args_size = process_vector_size(argc, argv) + process_vector_size(envc, envp) + sizeof(uint32_t) + 3;

    if(args_size > PROCESS_STACK_ARGS_LIMIT) {
        return NULL;
    }

    // Read the executable file

    file_stat_t executable_stat;
    file_descriptor_t* executable_fd;
    uint8_t* executable_data;

    if(file_stat(path, &executable_stat) < 0 || executable_stat.type != VFS_FILE) {
        return NULL;
    }

    if((executable_fd = file_open((char*) path, FILE_RDONLY, 0)) == NULL) {
        return NULL;
    }

    if((executable_data = (uint8_t*) kmalloc(executable_stat.size)) == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    if(file_read(executable_fd, executable_data, executable_stat.size) != executable_stat.size) {
        kfree(executable_data);
        file_close(executable_fd);
        return NULL;
    }

    file_close(executable_fd);

    // Create the process

    process_t* process = (process_t*) kmalloc(sizeof(process_t));

    if(!process) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    process->pid = process_next_pid();

    process->name = (char*) kmalloc(strlen(name) + 1);

    if(!process->name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(process->name, name);

    process->path = (char*) kmalloc(strlen(path) + 1);

    if(!process->path) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(process->path, path);

    process->state = PROCESS_STATE_READY;

    // Load the executable

    if(!elf_is_valid(executable_data, executable_stat.size)) {
        kfree(process->name);
        kfree(process->path);
        kfree(process);
        kfree(executable_data);
        return NULL;
    }

    page_directory_t* former_address_space = vmm_get_current_address_space();
    page_directory_t* address_space = vmm_create_address_space();

    if(address_space == NULL) {
        kfree(process->name);
        kfree(process->path);
        kfree(process);
        kfree(executable_data);
        return NULL;
    }

    process->address_space = address_space;

    // Temporarily switch to the new address space to load the executable
    vmm_switch_address_space(address_space);

    if(elf_load(executable_data, executable_stat.size) != 0) {
        kfree(process->name);
        kfree(process->path);
        kfree(process);
        kfree(executable_data);
        vmm_switch_address_space(former_address_space);
        return NULL;
    }

    // Allocate the user stack

    void* user_stack_limit = vmm_map_memory(NULL, PAGE_SIZE, NULL, false, true);

    if(user_stack_limit == NULL) {
        KPANIC(KPANIC_VMM_OUT_OF_USER_SPACE_MEMORY_CODE, KPANIC_VMM_OUT_OF_USER_SPACE_MEMORY_MESSAGE, NULL);
    }

    process->stack_base = (void*) ((uint32_t) user_stack_limit + PAGE_SIZE - 1);
    process->stack_limit = user_stack_limit;

    process->heap_base = NULL;
    process->heap_limit = NULL;

    /*
     * Build the initial user stack. The kernel copies the argument and environment strings onto
     * the stack and lays out the two pointer arrays so that, on entry, esp points at argc with
     * the argv array (argv[0] .. argv[argc - 1], NULL) directly above it and the envp array
     * (envp[0] .. envp[envc - 1], NULL) directly above that, the way the System V i386 ABI
     * specifies it. This has to happen while the new address space is active, as the stack
     * pages only exist there.
     */
    uint32_t user_esp = (uint32_t) user_stack_limit + PAGE_SIZE;

    uint32_t* arg_addresses = (uint32_t*) kmalloc((argc > 0 ? argc : 1) * sizeof(uint32_t));
    uint32_t* env_addresses = (uint32_t*) kmalloc((envc > 0 ? envc : 1) * sizeof(uint32_t));

    if(!arg_addresses || !env_addresses) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    // Copy each string onto the stack and remember its user-space address.
    for(int index = envc - 1; index >= 0; index--) {
        size_t length = strlen(envp[index]) + 1;
        user_esp -= length;
        memcpy((void*) user_esp, envp[index], length);
        env_addresses[index] = user_esp;
    }

    for(int index = argc - 1; index >= 0; index--) {
        size_t length = strlen(argv[index]) + 1;
        user_esp -= length;
        memcpy((void*) user_esp, argv[index], length);
        arg_addresses[index] = user_esp;
    }

    // Align the stack pointer before pushing pointer-sized values.
    user_esp &= ~0x3u;

    // NULL terminator of the envp array.
    user_esp -= sizeof(uint32_t);
    *((uint32_t*) user_esp) = 0;

    // envp pointers, highest index first so envp[0] ends up adjacent to the argv terminator.
    for(int index = envc - 1; index >= 0; index--) {
        user_esp -= sizeof(uint32_t);
        *((uint32_t*) user_esp) = env_addresses[index];
    }

    // NULL terminator of the argv array.
    user_esp -= sizeof(uint32_t);
    *((uint32_t*) user_esp) = 0;

    // argv pointers, highest index first so argv[0] ends up adjacent to argc.
    for(int index = argc - 1; index >= 0; index--) {
        user_esp -= sizeof(uint32_t);
        *((uint32_t*) user_esp) = arg_addresses[index];
    }

    // argc, sitting at the top of the stack on entry.
    user_esp -= sizeof(uint32_t);
    *((uint32_t*) user_esp) = (uint32_t) argc;

    kfree(arg_addresses);
    kfree(env_addresses);

    // Switch back to former address space after loading the executable
    vmm_switch_address_space(former_address_space);

    process->context.eip = (uint32_t) elf_get_entry_point(executable_data, executable_stat.size);
    process->context.esp = user_esp;
    process->context.eflags = 0x200;

    kfree(executable_data);

    // Create the streams

    process->out = out;
    process->in = in;
    process->err = err;

    // Initialize the file descriptors

    for(int index = 0; index < PROCESS_MAX_FILE_DESCRIPTORS; index++) {
        process->files[index] = NULL;
    }

    strncpy(process->cwd, cwd, PATH_MAX);
    process->cwd[PATH_MAX - 1] = '\0';

    // Initialize the parent relationship (set by the spawn syscall if any)

    process->parent = NULL;

    // Initialize the signals

    process->exit_code = 0;
    process->exception_code = -1;

    process_register(process);

    return process;
}

/**
 * Bytes a string vector takes on the initial stack: the strings themselves
 * plus the pointer array including its NULL terminator.
 */
static size_t process_vector_size(int count, const char** vector) {
    size_t size = (count + 1) * sizeof(uint32_t);

    for(int index = 0; index < count; index++) {
        size += strlen(vector[index]) + 1;
    }

    return size;
}

void process_destroy(process_t* process) {
    process_unregister(process);

    vmm_destroy_address_space(process->address_space);
    kfree(process->name);
    kfree(process->path);
    kfree(process);
}

void process_run(process_t* process) {
    if(process->state != PROCESS_STATE_READY) {
        return;
    }

    current_process = process;

    process->state = PROCESS_STATE_RUNNING;

    // Switch to the new address space
    vmm_switch_address_space(process->address_space);

    const uint32_t USER_DS_SELECTOR = 0x23;
    const uint32_t USER_CS_SELECTOR = 0x1B;

    __asm__ volatile (
        "cli\n"
        "mov %0, %%ds\n"
        "mov %0, %%es\n"
        "mov %0, %%fs\n"
        "mov %0, %%gs\n"
        "pushl %1\n" // User Stack Segment
        "pushl %2\n" // User Stack Pointer
        "pushl %3\n" // EFLAGS
        "pushl %4\n" // User Code Segment
        "pushl %5\n" // EIP
        "iret\n"
        :
        : "r"(USER_DS_SELECTOR),
          "r"(USER_DS_SELECTOR),
          "r"(process->context.esp),
          "r"(process->context.eflags),
          "r"(USER_CS_SELECTOR),
          "r"(process->context.eip)
    );
}

void process_terminate(process_t* process) {
    int32_t exit_code = process->exit_code;
    int32_t exception_code = process->exception_code;
    process_t* parent = process->parent;

    process->state = PROCESS_STATE_EXITED;

    /*
     * Destroy the exiting process while its own address space is still the
     * active one, so that vmm_destroy_address_space tears down the exiting
     * process' user space (it operates on the current address space) and not
     * some other process'. This leaves the kernel page directory active.
     */
    process_destroy(process);

    if(parent != NULL) {
        /*
         * The process was spawned by a waiting userland parent. Switch into the
         * parent's address space and resume it right after its spawn syscall,
         * handing it the child's exit code as the syscall return value. A child
         * that faulted is reported as -1.
         */
        vmm_switch_address_space(parent->address_space);

        current_process = parent;
        parent->state = PROCESS_STATE_RUNNING;

        /*
         * Encode the child's outcome as the spawn syscall's return value. It is
         * always non-negative: a normal exit code (masked to a byte) or, for a
         * process terminated by a CPU exception, 128 + the exception number.
         * This lets the caller reserve negative values for "could not execute".
         */
        if(exception_code != -1) {
            parent->saved_state.eax = (uint32_t) (128 + exception_code);
        } else {
            parent->saved_state.eax = (uint32_t) (exit_code & 0xFF);
        }

        context_restore(&parent->saved_state);

        // context_restore does not return.
    }

    /*
     * A process without a userland parent is the init process (PID 1). It is
     * expected to run forever, so its termination is a fatal condition.
     */
    KPANIC(KPANIC_INIT_DIED_CODE, KPANIC_INIT_DIED_MESSAGE, NULL);
}

void process_kill_current(int32_t exit_code) {
    if(current_process == NULL) {
        return;
    }

    current_process->exit_code = exit_code;
    current_process->exception_code = -1;

    // Resumes the parent via process_terminate; does not return here.
    process_terminate(current_process);
}

const process_t* process_get_current() {
    return current_process;
}

const process_t* process_get_by_pid(pid_t pid) {
    if(process_list == NULL) {
        return NULL;
    }

    linked_list_node_t* node = linked_list_find(process_list, process_compare_pid, &pid);

    return node != NULL ? (process_t*) node->data : NULL;
}

/* PID 0 is never handed out; it is free to mean "no process". */
static pid_t process_next_pid() {
    static pid_t pid = 1;

    return pid++;
}

static void process_register(process_t* process) {
    if(process_list == NULL && (process_list = linked_list_create()) == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    linked_list_node_t* node = linked_list_create_node(process);

    if(node == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    linked_list_append(process_list, node);
}

static void process_unregister(process_t* process) {
    linked_list_foreach(process_list, node) {
        if(node->data == process) {
            linked_list_remove(process_list, node);
            kfree(node);
            return;
        }
    }
}

static bool process_compare_pid(void* node_data, void* compare_data) {
    return ((process_t*) node_data)->pid == *((pid_t*) compare_data);
}
