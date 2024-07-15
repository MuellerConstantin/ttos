[BITS 32]

[SECTION .text]

[GLOBAL tss_flush]

tss_flush:

    mov eax, 0x2B
    ltr ax
    ret
