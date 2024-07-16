[BITS 32]

[SECTION .text]

[GLOBAL inb]
[GLOBAL outb]

inb:

    push ebp
    mov ebp, esp

    mov dx, [esp + 8]   ; Set I/O port address
    in al, dx           ; Read data from port

    mov esp, ebp
    pop ebp

    ret

outb:

    push ebp
    mov ebp, esp

    mov al, [esp + 12]  ; Move data to be sent to I/O port
    mov dx, [esp + 8]   ; Set I/O port address
    out dx, al          ; Send data to port

    mov esp, ebp
    pop ebp

    ret

inw:

    push ebp
    mov ebp, esp

    mov dx, [esp + 8]   ; Set I/O port address
    in ax, dx           ; Read data from port

    mov esp, ebp
    pop ebp

    ret

outw:

    push ebp
    mov ebp, esp

    mov ax, [esp + 12]  ; Move data to be sent to I/O port
    mov dx, [esp + 8]   ; Set I/O port address
    out dx, ax          ; Send data to port

    mov esp, ebp
    pop ebp

    ret
