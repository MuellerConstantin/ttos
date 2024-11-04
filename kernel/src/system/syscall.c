#include <system/syscall.h>
#include <arch/i386/isr.h>
#include <io/stream.h>
#include <system/process.h>

static void syscall_handler(isr_cpu_state_t *state);

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
 * @param state CPU state
 * @return The number of bytes written or -1 on error.
 */
static int32_t syscall_write(isr_cpu_state_t *state);

void syscall_init() {
    isr_register_listener(SYSCALL_INTERRUPT, syscall_handler);
}

static void syscall_handler(isr_cpu_state_t *state) {
    uint32_t syscall = state->eax;

    switch(syscall) {
        case 1: {
            state->eax = syscall_write(state);
            break;
        }
        default: {
            break;
        }
    }
}

static int32_t syscall_write(isr_cpu_state_t *state) {
    int32_t fd = state->ebx;
    const uint8_t* buffer = (const uint8_t*) state->ecx;
    size_t size = state->edx;

    process_t* current_process = process_get_current();

    if(current_process && current_process->out && fd == 1) {
        char* message = kmalloc(size + 1);
        memcpy(message, buffer, size);
        message[size] = '\0';

        stream_puts(current_process->out, message);

        kfree(buffer);
        return size;
    }

    if(current_process && current_process->err && fd == 2) {
        char* message = kmalloc(size + 1);
        memcpy(message, buffer, size);
        message[size] = '\0';

        stream_puts(current_process->err, message);

        kfree(buffer);
        return size;
    }

    return -1;
}
