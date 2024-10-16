[BITS 32]

[SECTION .text]

[GLOBAL isr_sti]
[GLOBAL isr_cli]

isr_sti:

    sti
    ret

isr_cli:

    cli
    ret
