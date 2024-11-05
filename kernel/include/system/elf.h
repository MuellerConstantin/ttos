#ifndef _KERNEL_SYSTEM_ELF_H
#define _KERNEL_SYSTEM_ELF_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ELF_HEADER_MAGIC0 0x7F
#define ELF_HEADER_MAGIC1 'E'
#define ELF_HEADER_MAGIC2 'L'
#define ELF_HEADER_MAGIC3 'F'

#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

#define ELF_DATA_LSB 1 // Little Endian
#define ELF_DATA_MSB 2 // Big Endian

#define ELF_CURRENT_VERSION 1

#define ELF_TYPE_RELOC 1
#define ELF_TYPE_EXEC 2
#define ELF_TYPE_SHARED 3
#define ELF_TYPE_CORE 4

#define ELF_MACHINE_X86 3
#define ELF_MACHINE_X86_64 62

#define ELF_PROGRAM_TYPE_LOAD 1
#define ELF_PROGRAM_TYPE_DYNAMIC 2
#define ELF_PROGRAM_TYPE_INTERP 3
#define ELF_PROGRAM_TYPE_NOTE 4

#define ELF_PROGRAM_FLAG_EXEC 0x01
#define ELF_PROGRAM_FLAG_WRITE 0x02
#define ELF_PROGRAM_FLAG_READ 0x04

typedef enum {
    ELF_IDENT_MAGIC0,
    ELF_IDENT_MAGIC1,
    ELF_IDENT_MAGIC2,
    ELF_IDENT_MAGIC3,
    ELF_IDENT_CLASS,
    ELF_IDENT_DATA,
    ELF_IDENT_VERSION,
    ELF_IDENT_OSABI,
    ELF_IDENT_PADDING
} elf_indent_t;

typedef struct elf_header elf_header_t;

struct elf_header {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed));

typedef struct elf_program_header elf_program_header_t;

struct elf_program_header {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed));

/**
 * Validates if a given buffer is a valid and supported ELF file.
 * 
 * @param data The buffer to validate.
 * @param size The size of the buffer.
 * 
 * @return true if the buffer is a valid ELF file, false otherwise.
 */
bool elf_is_valid(uint8_t* data, size_t size);

/**
 * Returns the entry point of an ELF file.
 * 
 * @param data The buffer containing the ELF file.
 * @param size The size of the buffer.
 * 
 * @return The entry point of the ELF file.
 */
void* elf_get_entry_point(uint8_t* data, size_t size);

/**
 * Loads an ELF file into the current virtual address space.
 * 
 * @param data The buffer containing the ELF file.
 * @param size The size of the buffer.
 * @return 0 on success, -1 on error.
 */
int32_t elf_load(uint8_t* data, size_t size);

#endif // _KERNEL_SYSTEM_ELF_H
