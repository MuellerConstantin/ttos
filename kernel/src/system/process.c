#include <system/process.h>
#include <io/file.h>
#include <system/kpanic.h>
#include <system/elf.h>
#include <memory/kheap.h>

static process_t* current_process = NULL;

static pid_t process_next_pid();

process_t* process_create(const char* name, const char* path, stream_t* out, stream_t* in, stream_t* err) {
    // Read the executable file

    file_stat_t executable_stat;
    file_descriptor_t* executable_fd;
    uint8_t* executable_data;

    if(file_stat(path, &executable_stat) < 0) {
        return NULL;
    }

    if((executable_fd = file_open(path, FILE_RDONLY)) < 0) {
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

    // Switch back to former address space after loading the executable
    vmm_switch_address_space(former_address_space);

    process->context.eip = (uint32_t) elf_get_entry_point(executable_data, executable_stat.size);
    process->context.esp = (uint32_t) process->stack_base;
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

    return process;
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

const process_t* process_get_current() {
    return current_process;
}

static pid_t process_next_pid() {
    static pid_t pid = 0;

    return pid++;
}
