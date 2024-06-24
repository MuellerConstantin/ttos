[BITS 32]

[SECTION .text]

[GLOBAL interrupt_enable]
[GLOBAL interrupt_disable]

interrupt_enable:

    sti

    ret

interrupt_disable:

    cli

    ret
