[BITS 32]

[SECTION .text]

[GLOBAL switch_usermode]

switch_usermode:
    cli

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push 0x23
    push eax
    pushf
    push 0x1B
    push usermode_entry
    iret

usermode_entry:
    ret
