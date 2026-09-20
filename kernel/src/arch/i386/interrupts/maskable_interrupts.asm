[BITS 32]

[SECTION .text]

[GLOBAL isr_sti]
[GLOBAL isr_cli]
[GLOBAL isr_wait_interrupt]

isr_sti:

    sti
    ret

isr_cli:

    cli
    ret

isr_wait_interrupt:

    sti
    hlt
    cli
    ret
