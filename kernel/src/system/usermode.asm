[BITS 32]

[SECTION .text]

[GLOBAL usermode_switch]

usermode_switch:

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23
    push esp
    pushf
    push 0x1B
    lea eax, [usermode_entry]
    push eax
    iret

usermode_entry:

    add esp, 4
    ret
