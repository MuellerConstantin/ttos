#include <system/syscall.h>
#include <arch/i386/isr.h>
#include <io/tty.h>

static void syscall_handler(isr_cpu_state_t *state);
static void syscall_print(isr_cpu_state_t *state);

void syscall_init() {
    isr_register_listener(SYSCALL_INTERRUPT, syscall_handler);
}

static void syscall_handler(isr_cpu_state_t *state) {
    uint32_t syscall = state->eax;

    switch(syscall) {
        case 0: {
            syscall_print(state);
            break;
        }
        default: {
            break;
        }
    }

    state->eax = 0;
}

static void syscall_print(isr_cpu_state_t *state) {
    const char* message = (const char*) state->ebx;

    char* buffer = kmalloc(strlen(message) + 1);
    strcpy(buffer, message);

    tty_t* tty0 = tty_get_stdterm();

    if(tty0) {
        tty_puts(tty0, buffer);
    }

    kfree(buffer);
}
