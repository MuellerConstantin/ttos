#ifndef _KERNEL_H
#define _KERNEL_H

#define KERNEL_VIRTUAL_BASE 0xC0000000

extern const uint32_t kernel_physical_start;
extern const uint32_t kernel_physical_end;
extern const uint32_t kernel_virtual_start;
extern const uint32_t kernel_virtual_end;

#endif // _KERNEL_H
