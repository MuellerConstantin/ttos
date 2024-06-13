[BITS 32]

[SECTION .text]

[GLOBAL idt_flush]

idt_flush:

    mov eax, [esp + 0x04]   ; IDT pointer passed as a parameter
    lidt [eax]              ; Load passed IDT

    ret

