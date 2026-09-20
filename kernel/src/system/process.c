#include <system/process.h>
#include <arch/i386/isr.h>
#include <arch/i386/tss.h>
#include <arch/i386/context_switch.h>
#include <io/file.h>
#include <system/kpanic.h>
#include <system/elf.h>
#include <memory/kheap.h>
#include <util/string.h>
#include <util/linked_list.h>

/* The process on the CPU, or NULL while the idle context runs. */
static process_t* current_process = NULL;

/* Every process that exists, from process_create until process_destroy. */
static linked_list_t* process_list = NULL;

/* Stack pointer of the idle context (kmain on the boot stack) while a process runs. */
static uint32_t idle_esp = 0;

/* Set by the timer once the running process' time slice is used up. */
static volatile bool need_resched = false;

static pid_t process_next_pid();

static void process_enter();
static process_t* process_pick_next();

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

    // Allocate the kernel stack

    process->kernel_stack = kmalloc(PROCESS_KERNEL_STACK_SIZE);

    if(!process->kernel_stack) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    /*
     * Lay out the kernel stack the way context_switch expects to find it, so
     * that the first switch to this process pops four registers and returns
     * into process_enter, which performs the ring 3 entry.
     */
    uint32_t* kernel_esp = (uint32_t*) ((uintptr_t) process->kernel_stack + PROCESS_KERNEL_STACK_SIZE);

    *--kernel_esp = (uint32_t) process_enter;   // ret target of context_switch
    *--kernel_esp = 0;                          // ebp
    *--kernel_esp = 0;                          // ebx
    *--kernel_esp = 0;                          // esi
    *--kernel_esp = 0;                          // edi

    process->kernel_esp = (uint32_t) kernel_esp;

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

    kfree(process->kernel_stack);
    kfree(process->name);
    kfree(process->path);
    kfree(process);
}

/*
 * First code a process runs on its own kernel stack, reached through the ret
 * of context_switch. Enters ring 3 with the state process_create prepared: an
 * iret frame is built by hand because there is no interrupt to return from.
 */
static void process_enter() {
    process_t* process = current_process;

    const uint32_t USER_DS_SELECTOR = 0x23;
    const uint32_t USER_CS_SELECTOR = 0x1B;

    __asm__ volatile (
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

void process_schedule() {
    process_t* previous = current_process;
    process_t* next = process_pick_next();

    if(next == NULL) {
        // Nothing else to run: stay where we are unless the current process gave up the CPU.
        if(previous == NULL || previous->state == PROCESS_STATE_RUNNING) {
            return;
        }

        /*
         * Fall back to the idle context. Its page directory does not matter,
         * the kernel half is the same everywhere, and it never leaves ring 0,
         * so the TSS stays as it is.
         */
        current_process = NULL;

        context_switch(&previous->kernel_esp, idle_esp);

        return;
    }

    if(previous != NULL && previous->state == PROCESS_STATE_RUNNING) {
        previous->state = PROCESS_STATE_READY;
    }

    next->state = PROCESS_STATE_RUNNING;
    next->ticks_left = PROCESS_TIME_SLICE_TICKS;
    current_process = next;

    /*
     * The next process' ring 0 entries have to land on its own kernel stack.
     * The stack lives on the kernel heap, which is mapped identically in every
     * address space, so the page directory can be switched before the stack.
     */
    vmm_switch_address_space(next->address_space);
    tss_update_ring0_stack(0x10, (uintptr_t) next->kernel_stack + PROCESS_KERNEL_STACK_SIZE);

    context_switch(previous != NULL ? &previous->kernel_esp : &idle_esp, next->kernel_esp);
}

void process_tick() {
    if(current_process == NULL) {
        return;
    }

    if(current_process->ticks_left > 0) {
        current_process->ticks_left--;
    }

    if(current_process->ticks_left == 0) {
        need_resched = true;
    }
}

void process_preempt() {
    if(!need_resched || current_process == NULL) {
        return;
    }

    need_resched = false;

    process_schedule();
}

void process_block() {
    current_process->state = PROCESS_STATE_WAITING;

    process_schedule();
}

void process_wake(process_t* process) {
    if(process->state == PROCESS_STATE_WAITING) {
        process->state = PROCESS_STATE_READY;
    }
}

void process_exit(int32_t exit_code, int32_t exception_code) {
    process_t* process = current_process;

    /*
     * A process without a userland parent is the init process (PID 1). It is
     * expected to run forever, so its termination is a fatal condition.
     */
    if(process->parent == NULL) {
        KPANIC(KPANIC_INIT_DIED_CODE, KPANIC_INIT_DIED_MESSAGE, NULL);
    }

    process->exit_code = exit_code;
    process->exception_code = exception_code;
    process->state = PROCESS_STATE_EXITED;

    /*
     * Release the address space while it is still the active one, as
     * vmm_destroy_address_space tears down the user half of the current
     * address space. This leaves the kernel page directory active. The kernel
     * stack we are running on is heap memory and stays valid.
     */
    vmm_destroy_address_space(process->address_space);
    process->address_space = NULL;

    process_wake(process->parent);

    process_schedule();

    // Not reached: an exited process is never picked again.
}

void process_idle() {
    isr_cli();

    while(true) {
        process_schedule();
        isr_wait_interrupt();
    }
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

/*
 * Round robin: the first READY process after the current one in table order,
 * wrapping around to the start. NULL if no process is ready.
 */
static process_t* process_pick_next() {
    if(process_list == NULL) {
        return NULL;
    }

    linked_list_node_t* start = process_list->head;

    if(current_process != NULL) {
        linked_list_foreach(process_list, node) {
            if(node->data == current_process) {
                start = node->next;
                break;
            }
        }
    }

    for(linked_list_node_t* node = start; node != NULL; node = node->next) {
        if(((process_t*) node->data)->state == PROCESS_STATE_READY) {
            return (process_t*) node->data;
        }
    }

    for(linked_list_node_t* node = process_list->head; node != start; node = node->next) {
        if(((process_t*) node->data)->state == PROCESS_STATE_READY) {
            return (process_t*) node->data;
        }
    }

    return NULL;
}
