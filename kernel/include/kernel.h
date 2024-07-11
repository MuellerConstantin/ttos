#ifndef _KERNEL_H
#define _KERNEL_H

/** Minimal required RAM size. */
#define KERNEL_MINIMAL_RAM_SIZE 0x40000000

/** Virtual base address of the kernel space. */
#define KERNEL_SPACE_BASE 0xC0000000

/** Size of the kernel space. */
#define KERNEL_SPACE_SIZE 0x300000

extern const uint32_t kernel_physical_start;
extern const uint32_t kernel_physical_end;
extern const uint32_t kernel_virtual_start;
extern const uint32_t kernel_virtual_end;

#endif // _KERNEL_H
