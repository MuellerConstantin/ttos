#include <sys/kpanic.h>
#include <stdlib.h>
#include <string.h>

void kpanic(uint32_t code, const char *message, const char* file, uint32_t line, isr_cpu_state_t *state) {
    vga_init(VGA_80x25_16_TEXT, false);
    vga_tm_init();

    vga_tm_set_color(VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_fill(VGA_TM_BLUE);
    vga_tm_disable_cursor();

    char code_text[10];
    char line_text[10];

    vga_tm_putstr("KERNEL PANIC:\n");
    vga_tm_putstr("=============\n\n");
    vga_tm_putstr("An error occurred and the kernel has stopped working. The system has been halted to prevent damage. Please restart the system. If the error persists, please contact the system administrator.\n\n");

    vga_tm_putstr("EXCEPTION DETAILS:\n");
    vga_tm_putstr("------------------\n\n");

    vga_tm_putchar('"');
    vga_tm_putstr(message);
    vga_tm_putchar('"');

    vga_tm_putstr(" (0x");
    vga_tm_putstr(itoa(code, code_text, 16));
    vga_tm_putstr(")\n\n");

    // Print technical details

    vga_tm_putstr("TECHNICAL DETAILS:\n");
    vga_tm_putstr("------------------\n\n");

    vga_tm_putstr("File: ");
    vga_tm_putstr(file);
    vga_tm_putchar(':');
    vga_tm_putstr(itoa(line, line_text, 10));
    vga_tm_putstr("\n\n");

    if(NULL != state) {
        int spacer_length = 0;
        char register_value_text[9];

        // Print technical details: ESP + EBP

        vga_tm_putstr("ESP:    0x");
        vga_tm_putstr(itoa(state->esp, register_value_text, 16));

        spacer_length = 1 + 8 - strlen(register_value_text);

        for(int i = 0; i < spacer_length; i++) {
            vga_tm_putchar(' ');
        }

        vga_tm_putstr("EBP:    0x");
        vga_tm_putstr(itoa(state->ebp, register_value_text, 16));
        vga_tm_putstr("\n");

        // Print technical details: EIP + EFLAGS

        vga_tm_putstr("EIP:    0x");
        vga_tm_putstr(itoa(state->eip, register_value_text, 16));

        spacer_length = 1 + 8 - strlen(register_value_text);

        for(int i = 0; i < spacer_length; i++) {
            vga_tm_putchar(' ');
        }

        vga_tm_putstr("EFLAGS: 0x");
        vga_tm_putstr(itoa(state->eflags, register_value_text, 16));
        vga_tm_putstr("\n");

        // Print technical details: EAX + EBX

        vga_tm_putstr("EAX:    0x");
        vga_tm_putstr(itoa(state->eax, register_value_text, 16));

        spacer_length = 1 + 8 - strlen(register_value_text);

        for(int i = 0; i < spacer_length; i++) {
            vga_tm_putchar(' ');
        }

        vga_tm_putstr("EBX:    0x");
        vga_tm_putstr(itoa(state->ebx, register_value_text, 16));
        vga_tm_putstr("\n");

        // Print technical details: ECX + EDX

        vga_tm_putstr("ECX:    0x");
        vga_tm_putstr(itoa(state->ecx, register_value_text, 16));

        spacer_length = 1 + 8 - strlen(register_value_text);

        for(int i = 0; i < spacer_length; i++) {
            vga_tm_putchar(' ');
        }

        vga_tm_putstr("EDX:    0x");
        vga_tm_putstr(itoa(state->edx, register_value_text, 16));
        vga_tm_putstr("\n");

        // Print technical details: ESI + EDI
        
        vga_tm_putstr("ESI:    0x");
        vga_tm_putstr(itoa(state->esi, register_value_text, 16));

        spacer_length = 1 + 8 - strlen(register_value_text);

        for(int i = 0; i < spacer_length; i++) {
            vga_tm_putchar(' ');
        }

        vga_tm_putstr("EDI:    0x");
        vga_tm_putstr(itoa(state->edi, register_value_text, 16));
        vga_tm_putstr("\n");

        // Print technical details: CS + SS

        vga_tm_putstr("CS:     0x");
        vga_tm_putstr(itoa(state->cs, register_value_text, 16));

        spacer_length = 1 + 8 - strlen(register_value_text);

        for(int i = 0; i < spacer_length; i++) {
            vga_tm_putchar(' ');
        }

        vga_tm_putstr("SS:     0x");
        vga_tm_putstr(itoa(state->ss, register_value_text, 16));
        vga_tm_putstr("\n");

        // Print technical details: DS

        vga_tm_putstr("DS:     0x");
        vga_tm_putstr(itoa(state->ds, register_value_text, 16));
        vga_tm_putstr("\n");
    }

    // Halt the system
    while(1);
}
