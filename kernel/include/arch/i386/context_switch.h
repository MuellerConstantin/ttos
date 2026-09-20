/**
 * @file context_switch.h
 * @brief Kernel stack switch between execution contexts.
 */

#ifndef _KERNEL_ARCH_I386_CONTEXT_SWITCH_H
#define _KERNEL_ARCH_I386_CONTEXT_SWITCH_H

#include <stdint.h>

/**
 * Switches from the current kernel stack to another one. The callee-saved
 * registers are pushed onto the current stack and its stack pointer is stored
 * through prev_esp; then next_esp becomes the stack pointer and execution
 * resumes wherever that stack was last left. The call returns once a later
 * switch loads the stack pointer stored through prev_esp again.
 *
 * Interrupts have to be disabled by the caller. Page directory and ring 0
 * stack pointer of the TSS are not touched; they have to be set up for the
 * next context before calling.
 *
 * @param prev_esp Where to store the current stack pointer.
 * @param next_esp The stack pointer to switch to.
 */
extern void context_switch(uint32_t* prev_esp, uint32_t next_esp);

#endif // _KERNEL_ARCH_I386_CONTEXT_SWITCH_H
