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

/*
 * Size of the kernel stack each process runs on while it is inside the kernel.
 * There is no guard page; an overflow corrupts the neighbouring heap block.
 */
#define PROCESS_KERNEL_STACK_SIZE 8192

/* Timer ticks a process may run before it is preempted, at the PIT's 100 Hz. */
#define PROCESS_TIME_SLICE_TICKS 10

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
 * waiting to be destroyed by its parent.
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

    /* Ring 3 entry state, used once when the process first runs. */
    process_context_t context;

    /*
     * The kernel stack this process runs on inside the kernel, and its stack
     * pointer while the process is not on the CPU. The stack holds whatever the
     * process was doing when it was switched away from, down to the ISR frame
     * through which it entered the kernel.
     */
    void* kernel_stack;
    uint32_t kernel_esp;

    /* Timer ticks left in the current time slice. */
    uint32_t ticks_left;

    /*
     * The process that spawned this one, or NULL for a process without a
     * userland parent (the init process launched by the kernel). The parent
     * collects the exit information with wait and destroys the child; a child
     * whose parent exits first is handed to init.
     */
    struct process* parent;

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
 * Destroy a process that has exited: releases its kernel stack, its bookkeeping
 * and its table entry. The address space has already been released by
 * process_exit. Called by the parent once it has read the exit information.
 *
 * @param process The process to destroy.
 */
void process_destroy(process_t* process);

/**
 * Hand the CPU to the next process that is ready to run, or to the kernel's
 * idle context if there is none. Returns once the calling context is scheduled
 * again. A caller that wants to give up the CPU marks its state before calling;
 * a process still marked RUNNING is kept running if nothing else is ready.
 *
 * Must be called with interrupts disabled.
 */
void process_schedule();

/**
 * Account one timer tick to the running process. When its time slice is used
 * up, a reschedule is requested for the next return to userland. Called from
 * the timer interrupt.
 */
void process_tick();

/**
 * Give the CPU to another ready process if a reschedule has been requested.
 * Called on the way out of an interrupt that returns to userland; the kernel
 * itself is never preempted. Returns once the current process is scheduled
 * again, or immediately if nothing else is ready.
 */
void process_preempt();

/**
 * Block the current process until it is woken by process_wake, and run
 * something else in the meantime. Returns once the process is resumed.
 */
void process_block();

/**
 * Mark a blocked process ready to run again. Has no effect on a process that
 * is not blocked.
 *
 * @param process The process to wake.
 */
void process_wake(process_t* process);

/**
 * Terminate the current process. Records the outcome, releases the address
 * space, hands any children over to init, wakes the parent and switches away
 * for good; the parent destroys what is left. May be called from a syscall or
 * from an interrupt handler (the interrupted kernel path is abandoned with the
 * process' kernel stack). Does not return. A process without a parent is the
 * init process; its exit is a kernel panic.
 *
 * @param exit_code The exit code delivered to the parent.
 * @param exception_code The CPU exception that terminated the process, or -1
 *                       for a normal exit.
 */
void process_exit(int32_t exit_code, int32_t exception_code);

/**
 * Run the kernel's idle context: schedules whenever something is ready and
 * halts the CPU in between. Never returns; the calling stack becomes the stack
 * of the idle context.
 */
void process_idle();

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

/**
 * Look for a child of a process. With a PID, that child in whatever state it
 * is; with -1, any child that has exited, or failing that any child at all.
 *
 * @param parent The parent.
 * @param pid The child's PID, or -1 for any child.
 * @return The child or NULL if the parent has no such child.
 */
process_t* process_find_child(const process_t* parent, pid_t pid);

#endif // _KERNEL_SYSTEM_PROCESS_H
