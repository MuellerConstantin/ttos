[BITS 32]

[SECTION .text]

[GLOBAL gdt_flush]

gdt_flush:

    mov eax, [esp + 4]      ; GDT pointer passed as a parameter
    lgdt [eax]              ; Load passed GDT

    jmp 0x08:.gdt_setup    ; Jump to linear address kernel code segment

.gdt_setup:

    mov ax, 0x10        ; Set offset of kernel data segment in GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret
