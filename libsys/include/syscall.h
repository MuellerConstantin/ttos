#ifndef _LIBSYS_SYSCALL_H
#define _LIBSYS_SYSCALL_H

#include <stdint.h>
#include <ttos/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The stubs have to end up inside their caller. libsys and the userland
 * programs are built without optimisation, and gcc ignores a plain inline at
 * that level, so without the attribute every system call would pay for an
 * additional call and return.
 */
#define SYSCALL_INLINE __attribute__((always_inline)) static inline

// Turns the interrupt vector into a literal the assembler template can use.
#define SYSCALL_STR_(value) #value
#define SYSCALL_STR(value) SYSCALL_STR_(value)

/**
 * Issues a system call without arguments.
 *
 * @param number The system call number.
 * @return The value the kernel left in eax.
 */
SYSCALL_INLINE int32_t syscall0(uint32_t number) {
    int32_t result;

    __asm__ volatile(
        "int $" SYSCALL_STR(SYSCALL_INTERRUPT) "\n"
        : "=a"(result)
        : "a"(number)
        : "memory"
    );

    return result;
}

/**
 * Issues a system call with a single argument, passed in ebx.
 *
 * @param number The system call number.
 * @param argument1 The first argument.
 * @return The value the kernel left in eax.
 */
SYSCALL_INLINE int32_t syscall1(uint32_t number, uint32_t argument1) {
    int32_t result;

    __asm__ volatile(
        "int $" SYSCALL_STR(SYSCALL_INTERRUPT) "\n"
        : "=a"(result)
        : "a"(number), "b"(argument1)
        : "memory"
    );

    return result;
}

/**
 * Issues a system call with two arguments, passed in ebx and ecx.
 *
 * @param number The system call number.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @return The value the kernel left in eax.
 */
SYSCALL_INLINE int32_t syscall2(uint32_t number, uint32_t argument1, uint32_t argument2) {
    int32_t result;

    __asm__ volatile(
        "int $" SYSCALL_STR(SYSCALL_INTERRUPT) "\n"
        : "=a"(result)
        : "a"(number), "b"(argument1), "c"(argument2)
        : "memory"
    );

    return result;
}

/**
 * Issues a system call with three arguments, passed in ebx, ecx and edx.
 *
 * @param number The system call number.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @return The value the kernel left in eax.
 */
SYSCALL_INLINE int32_t syscall3(uint32_t number, uint32_t argument1, uint32_t argument2, uint32_t argument3) {
    int32_t result;

    __asm__ volatile(
        "int $" SYSCALL_STR(SYSCALL_INTERRUPT) "\n"
        : "=a"(result)
        : "a"(number), "b"(argument1), "c"(argument2), "d"(argument3)
        : "memory"
    );

    return result;
}

/**
 * Issues a system call with four arguments.
 *
 * @param number The system call number.
 * @param argument1 The first argument.
 * @param argument2 The second argument.
 * @param argument3 The third argument.
 * @param argument4 The fourth argument.
 * @return The value the kernel left in eax.
 */
SYSCALL_INLINE int32_t syscall4(uint32_t number, uint32_t argument1, uint32_t argument2, uint32_t argument3, uint32_t argument4) {
    int32_t result;

    __asm__ volatile(
        "int $" SYSCALL_STR(SYSCALL_INTERRUPT) "\n"
        : "=a"(result)
        : "a"(number), "b"(argument1), "c"(argument2), "d"(argument3), "S"(argument4)
        : "memory"
    );

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // _LIBSYS_SYSCALL_H
