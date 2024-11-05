#include <system/elf.h>
#include <memory/vmm.h>

bool elf_is_valid(uint8_t* data, size_t size) {
    if(size < sizeof(elf_header_t)) {
        return false;
    }

    elf_header_t* header = (elf_header_t*) data;

    if(header->e_ident[ELF_IDENT_MAGIC0] != ELF_HEADER_MAGIC0 ||
       header->e_ident[ELF_IDENT_MAGIC1] != ELF_HEADER_MAGIC1 ||
       header->e_ident[ELF_IDENT_MAGIC2] != ELF_HEADER_MAGIC2 ||
       header->e_ident[ELF_IDENT_MAGIC3] != ELF_HEADER_MAGIC3
    ) {
        return false;
    }

    if(header->e_ident[ELF_IDENT_CLASS] != ELF_CLASS_32) {
        return false;
    }

    if(header->e_ident[ELF_IDENT_DATA] != ELF_DATA_LSB) {
        return false;
    }

    if(header->e_ident[ELF_IDENT_VERSION] != ELF_CURRENT_VERSION) {
        return false;
    }

    if(header->e_type != ELF_TYPE_EXEC) {
        return false;
    }

    if(header->e_machine != ELF_MACHINE_X86) {
        return false;
    }

    return true;
}

void* elf_get_entry_point(uint8_t* data, size_t size) {
    if(!elf_is_valid(data, size)) {
        return NULL;
    }

    elf_header_t* header = (elf_header_t*) data;

    return (void*) header->e_entry;
}

int32_t elf_load(uint8_t* data, size_t size) {
    if(!elf_is_valid(data, size)) {
        return -1;
    }

    elf_header_t* header = (elf_header_t*) data;
    elf_program_header_t* program_header = (elf_program_header_t*) (data + header->e_phoff);

    for(size_t index = 0; index < header->e_phnum; index++, program_header++) {
        if(program_header->p_type == ELF_PROGRAM_TYPE_LOAD) {
            bool is_writeable = (program_header->p_flags & ELF_PROGRAM_FLAG_WRITE) != 0;

            if(vmm_map_memory(program_header->p_vaddr, program_header->p_memsz, NULL, false, is_writeable) == NULL) {
                return -1;
            }

            memcpy((void*) program_header->p_vaddr, (void*) (data + program_header->p_offset), program_header->p_filesz);

            if (program_header->p_memsz > program_header->p_filesz) {
                memset((void*) (program_header->p_vaddr + program_header->p_filesz), 0, program_header->p_memsz - program_header->p_filesz);
            }
        }
    }

    return 0;
}
