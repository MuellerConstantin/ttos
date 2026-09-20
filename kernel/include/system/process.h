#ifndef _KERNEL_SYSTEM_PROCESS_H
#define _KERNEL_SYSTEM_PROCESS_H

#include <stdint.h>
#include <memory/vmm.h>
#include <io/stream.h>
#include <io/file.h>
#include <arch/i386/isr.h>
#include <ttos/syscall.h>

#define PROCESS_MAX_FILE_DESCRIPTORS 32

/*
 * Upper bound for what the arguments and the environment may take of the
 * initial user stack, which is a single page. The program has to run on what
 * is left, so the two together may not claim more than half of it.
 */
#define PROCESS_STACK_ARGS_LIMIT (PAGE_SIZE / 2)

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

/*
 * READY: created or resumable, not on the CPU. RUNNING: on the CPU. WAITING:
 * blocked until an event occurs (a spawned child exiting). EXITED: terminated,
 * about to be destroyed.
 */
typedef enum {
    PROCESS_STATE_READY = 0,
    PROCESS_STATE_RUNNING = 1,
    PROCESS_STATE_EXITED = 2,
    PROCESS_STATE_WAITING = 3
} process_state_t;

typedef struct process process_t;

struct process {
    pid_t pid;
    char* name;
    char* path;

    process_state_t state;

    page_directory_t* address_space;
    process_context_t context;

    /*
     * When this process spawns a child and blocks on it, its full CPU state at
     * the spawn syscall boundary is stored here so process_terminate can resume
     * it once the child exits. parent points at the process to resume, or NULL
     * for a process without a userland parent (the init process launched by the
     * kernel).
     */
    struct process* parent;
    isr_cpu_state_t saved_state;

    void* stack_base;
    void* stack_limit;

    void* heap_base;
    void* heap_limit;

    stream_t* out;
    stream_t* in;
    stream_t* err;

    file_descriptor_t* files[PROCESS_MAX_FILE_DESCRIPTORS];

    /*
     * Working directory, absolute and normalized (see path_resolve). Every
     * relative path this process hands to the kernel is resolved against it.
     */
    char cwd[PATH_MAX];

    int32_t exit_code;
    int32_t exception_code;
};

/**
 * Create a new process.
 * 
 * @param name Name of the process.
 * @param path Path to the executable.
 * @param argc Number of arguments passed to the process.
 * @param argv Argument vector (argv[0] is conventionally the program path).
 * @param envc Number of environment strings passed to the process.
 * @param envp Environment vector, each entry of the form NAME=VALUE.
 * @param cwd Working directory the process starts in, absolute and normalized.
 * @param out Output stream.
 * @param in Input stream.
 * @param err Error stream.
 * @return The new process or NULL if the executable could not be loaded or the
 *         arguments and environment do not fit onto the initial stack.
 */
process_t* process_create(const char* name, const char* path, int argc, const char** argv, int envc, const char** envp, const char* cwd, stream_t* out, stream_t* in, stream_t* err);

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
 * Terminate the currently running process from outside a syscall (e.g. from an
 * interrupt handler, for Ctrl+C) and resume its parent with the given exit code.
 * Does not return when the current process has a userland parent.
 *
 * @param exit_code The exit code delivered to the resumed parent.
 */
void process_kill_current(int32_t exit_code);

/**
 * Get the current process.
 * 
 * @return The current process.
 */
const process_t* process_get_current();

/**
 * Look up a process by its PID.
 *
 * @param pid The PID to look up.
 * @return The process or NULL if no process with that PID exists.
 */
const process_t* process_get_by_pid(pid_t pid);

#endif // _KERNEL_SYSTEM_PROCESS_H
