#ifndef _KERNEL_H
#define _KERNEL_H

#define KERNEL_VIRTUAL_BASE 0xC0000000

extern uint32_t kernel_physical_start;
extern uint32_t kernel_physical_end;
extern uint32_t kernel_virtual_start;
extern uint32_t kernel_virtual_end;

#endif // _KERNEL_H
