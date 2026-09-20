[BITS 32]

[SECTION .text]

; void context_switch(uint32_t* prev_esp, uint32_t next_esp)
;
; Switches kernel stacks. The callee-saved registers of the cdecl convention
; are pushed onto the current stack and its stack pointer is stored through
; prev_esp. Then esp is loaded from next_esp, the same registers are popped
; from the new stack and ret continues wherever that stack's owner last called
; context_switch, or, for a process that has never run, in the entry trampoline
; process_create placed there. Everything else the resumed code needs is either
; caller-saved or sits in the ISR frame further up its stack.
;
; Must be called with interrupts disabled: between the two stack pointer moves
; there is no stack an interrupt could safely use.

[GLOBAL context_switch]

context_switch:

    push ebp
    push ebx
    push esi
    push edi

    mov eax, [esp + 0x14]   ; prev_esp
    mov ecx, [esp + 0x18]   ; next_esp

    mov [eax], esp
    mov esp, ecx

    pop edi
    pop esi
    pop ebx
    pop ebp

    ret
