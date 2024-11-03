#ifndef _KERNEL_SYSTEM_PROCESS_H
#define _KERNEL_SYSTEM_PROCESS_H

#include <stdint.h>
#include <memory/vmm.h>

typedef int32_t pid_t;

typedef struct process_context process_context_t;

struct process_context {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
};

typedef struct process process_t;

struct process {
    pid_t pid;
    char* name;
    char* path;
    void* stack;
    page_directory_t* address_space;
    process_context_t context;
};

/**
 * Create a new process.
 * 
 * @param name Name of the process.
 * @param path Path to the executable.
 * @return The new process.
 */
process_t* process_create(const char* name, const char* path);

/**
 * Run a process.
 * 
 * @param process The process to run.
 */
void process_run(process_t* process);

#endif // _KERNEL_SYSTEM_PROCESS_H
