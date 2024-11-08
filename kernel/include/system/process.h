#ifndef _KERNEL_SYSTEM_PROCESS_H
#define _KERNEL_SYSTEM_PROCESS_H

#include <stdint.h>
#include <memory/vmm.h>
#include <io/stream.h>
#include <io/file.h>

#define PROCESS_MAX_FILE_DESCRIPTORS 32

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

typedef enum {
    PROCESS_STATE_READY = 0,
    PROCESS_STATE_RUNNING = 1,
    PROCESS_STATE_EXITED = 2
} process_state_t;

typedef struct process process_t;

struct process {
    pid_t pid;
    char* name;
    char* path;

    process_state_t state;

    page_directory_t* address_space;
    process_context_t context;

    void* stack_base;
    void* stack_limit;

    void* heap_base;
    void* heap_limit;

    stream_t* out;
    stream_t* in;
    stream_t* err;

    file_descriptor_t* files[PROCESS_MAX_FILE_DESCRIPTORS];
};

/**
 * Create a new process.
 * 
 * @param name Name of the process.
 * @param path Path to the executable.
 * @param out Output stream.
 * @param in Input stream.
 * @param err Error stream.
 * @return The new process.
 */
process_t* process_create(const char* name, const char* path, stream_t* out, stream_t* in, stream_t* err);

/**
 * Destroy a process.
 * 
 * @param process The process to destroy.
 */
void process_destroy(process_t* process);

/**
 * Run a process.
 * 
 * @param process The process to run.
 */
void process_run(process_t* process);

/**
 * Terminate a process.
 * 
 * @param process The process to terminate.
 */
void process_terminate(process_t* process);

/**
 * Get the current process.
 * 
 * @return The current process.
 */
const process_t* process_get_current();

#endif // _KERNEL_SYSTEM_PROCESS_H
