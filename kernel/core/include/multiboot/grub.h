#ifndef _KERNEL_CORE_MULTIBOOT_GRUB_H
#define _KERNEL_CORE_MULTIBOOT_GRUB_H

#include <stdint.h>

/*
 * GNU Grub multiboot specification (version 0.6.96) compliant header as interface between
 * boot loader and operating system.
 */

struct multiboot_header {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
} __attribute__ ((packed));

typedef struct multiboot_header multiboot_header_t;

#endif // _KERNEL_CORE_MULTIBOOT_GRUB_H
