#include <sys/kpanic.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_NEXT_LINE(screen_width, screen_offset) (screen_offset + (screen_width - (screen_offset % screen_width)))
#define SCREEN_NEXT_PARAGRAPH(screen_width, screen_offset) (SCREEN_NEXT_LINE(screen_width, screen_offset) + screen_width)

const char* PANIC_TITLE = "KERNEL PANIC:";
const char* PANIC_TITLE_UNDERLINE = "=============";
const char* USER_INTRODUCTION = "An error occurred and the kernel has stopped working. " \
    "The system has been halted to prevent damage. Please restart the system. " \
    "If the error persists, please contact the system administrator.";
const char* EXCEPTION_TITLE = "EXCEPTION DETAILS:";
const char* EXCEPTION_TITLE_UNDERLINE = "------------------";
const char* FILE_LABEL = "File:";
const char* TECHNICAL_TITLE = "TECHNICAL DETAILS:";
const char* TECHNICAL_TITLE_UNDERLINE = "------------------";

void kpanic(uint32_t code, const char *message, const char* file, uint32_t line, isr_cpu_state_t *state) {
    vga_init(VGA_80x25_16_TEXT);

    vga_tm_fill(VGA_TM_BLUE);
    vga_tm_disable_cursor();

    // Print user information and exception details

    const size_t panic_title_length = strlen(PANIC_TITLE);
    const size_t panic_title_screen_offset = 0;
    const size_t panic_title_screen_limit = panic_title_screen_offset + panic_title_length;

    const size_t panic_title_underline_screen_offset = SCREEN_NEXT_LINE(80, panic_title_screen_limit);
    const size_t panic_title_underline_screen_limit = panic_title_underline_screen_offset + panic_title_length;

    const size_t user_introduction_length = strlen(USER_INTRODUCTION);
    const size_t user_introduction_screen_offset = SCREEN_NEXT_PARAGRAPH(80, panic_title_underline_screen_limit);
    const size_t user_introduction_screen_limit = user_introduction_screen_offset + user_introduction_length;

    const size_t exception_title_length = strlen(EXCEPTION_TITLE);
    const size_t exception_title_screen_offset = SCREEN_NEXT_PARAGRAPH(80, user_introduction_screen_limit);
    const size_t exception_title_screen_limit = exception_title_screen_offset + exception_title_length;

    const size_t exception_title_underline_screen_offset = SCREEN_NEXT_LINE(80, exception_title_screen_limit);
    const size_t exception_title_underline_screen_limit = exception_title_underline_screen_offset + exception_title_length;

    const size_t message_screen_length = strlen(message) + 2;
    const size_t message_screen_offset = SCREEN_NEXT_PARAGRAPH(80, exception_title_underline_screen_limit);
    const size_t message_screen_limit = message_screen_offset + message_screen_length;

    char code_text[10];

    itoa(code, code_text, 16);

    const size_t code_text_length = strlen(code_text) + 4;
    const size_t code_text_screen_offset = message_screen_limit + 1;
    const size_t code_text_screen_limit = code_text_screen_offset + code_text_length;

    vga_tm_strwrite(panic_title_screen_offset, PANIC_TITLE, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(panic_title_underline_screen_offset, PANIC_TITLE_UNDERLINE, VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(user_introduction_screen_offset, USER_INTRODUCTION, VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(exception_title_screen_offset, EXCEPTION_TITLE, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(exception_title_underline_screen_offset, EXCEPTION_TITLE_UNDERLINE, VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(message_screen_offset, "\"", VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(message_screen_offset + 1, message, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(message_screen_limit - 1, "\"", VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(code_text_screen_offset, "(0x", VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(code_text_screen_offset + 3, code_text, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(code_text_screen_limit - 1, ")", VGA_TM_WHITE, VGA_TM_BLUE);

    // Print technical details

    const size_t technical_title_length = strlen(TECHNICAL_TITLE);
    const size_t technical_title_screen_offset = SCREEN_NEXT_PARAGRAPH(80, code_text_screen_limit);
    const size_t technical_title_screen_limit = technical_title_screen_offset + technical_title_length;

    const size_t technical_title_underline_screen_offset = SCREEN_NEXT_LINE(80, technical_title_screen_limit);
    const size_t technical_title_underline_screen_limit = technical_title_underline_screen_offset + technical_title_length;

    vga_tm_strwrite(technical_title_screen_offset, TECHNICAL_TITLE, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(technical_title_underline_screen_offset, TECHNICAL_TITLE_UNDERLINE, VGA_TM_WHITE, VGA_TM_BLUE);

    const size_t file_label_length = strlen(FILE_LABEL);
    const size_t file_label_screen_offset = SCREEN_NEXT_PARAGRAPH(80, technical_title_underline_screen_limit);
    const size_t file_label_screen_limit = file_label_screen_offset + file_label_length;

    const size_t file_length = strlen(file);
    const size_t file_screen_offset = file_label_screen_limit + 1;
    const size_t file_screen_limit = file_screen_offset + file_length;

    char line_text[10];

    itoa(line, line_text, 10);

    const size_t line_text_length = strlen(line_text) + 1;
    const size_t line_text_screen_offset = file_screen_limit;
    const size_t line_text_screen_limit = line_text_screen_offset + line_text_length;

    vga_tm_strwrite(file_label_screen_offset, FILE_LABEL, VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(file_screen_offset, file, VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(line_text_screen_offset, ":", VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(line_text_screen_offset + 1, line_text, VGA_TM_WHITE, VGA_TM_BLUE);

    if(NULL != state) {

        // Print technical details: ESP + EBP

        const char* esp_name = "ESP:    0x";
        char esp_value[9];

        const char* ebp_name = "EBP:    0x";
        char ebp_value[9];

        itoa(state->esp, esp_value, 16);
        itoa(state->ebp, ebp_value, 16);

        const size_t esp_ebp_spacer = 8 - strlen(esp_value);

        const size_t esp_name_length = strlen(esp_name);
        const size_t esp_value_length = strlen(esp_value);
        const size_t ebp_name_length = strlen(ebp_name);
        const size_t ebp_value_length = strlen(ebp_value);

        const size_t esp_screen_offset = SCREEN_NEXT_PARAGRAPH(80, line_text_screen_limit);
        const size_t esp_screen_limit = esp_screen_offset + esp_name_length + esp_value_length + esp_ebp_spacer;

        const size_t ebp_screen_offset = esp_screen_limit + 1;
        const size_t ebp_screen_limit = ebp_screen_offset + ebp_name_length + ebp_value_length;

        vga_tm_strwrite(esp_screen_offset, esp_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(esp_screen_offset + esp_name_length, esp_value, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(ebp_screen_offset, ebp_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(ebp_screen_offset + ebp_name_length, ebp_value, VGA_TM_WHITE, VGA_TM_BLUE);

        // Print technical details: EIP + EFLAGS

        const char* eip_name = "EIP:    0x";
        char eip_value[9];

        const char* eflags_name = "EFLAGS: 0x";
        char eflags_value[9];

        itoa(state->eip, eip_value, 16);
        itoa(state->eflags, eflags_value, 16);

        const size_t eip_eflags_spacer = 8 - strlen(eip_value);

        const size_t eip_name_length = strlen(eip_name);
        const size_t eip_value_length = strlen(eip_value);
        const size_t eflags_name_length = strlen(eflags_name);
        const size_t eflags_value_length = strlen(eflags_value);

        const size_t eip_screen_offset = SCREEN_NEXT_LINE(80, ebp_screen_limit);
        const size_t eip_screen_limit = eip_screen_offset + eip_name_length + eip_value_length + eip_eflags_spacer;

        const size_t eflags_screen_offset = eip_screen_limit + 1;
        const size_t eflags_screen_limit = eflags_screen_offset + eflags_name_length + eflags_value_length;

        vga_tm_strwrite(eip_screen_offset, eip_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(eip_screen_offset + eip_name_length, eip_value, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(eflags_screen_offset, eflags_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(eflags_screen_offset + eflags_name_length, eflags_value, VGA_TM_WHITE, VGA_TM_BLUE);

        // Print technical details: EAX + EBX

        const char* eax_name = "EAX:    0x";
        char eax_value[9];

        const char* ebx_name = "EBX:    0x";
        char ebx_value[9];

        itoa(state->eax, eax_value, 16);
        itoa(state->ebx, ebx_value, 16);

        const size_t eax_ebx_spacer = 8 - strlen(eax_value);

        const size_t eax_name_length = strlen(eax_name);
        const size_t eax_value_length = strlen(eax_value);
        const size_t ebx_name_length = strlen(ebx_name);
        const size_t ebx_value_length = strlen(ebx_value);

        const size_t eax_screen_offset = SCREEN_NEXT_LINE(80, eflags_screen_limit);
        const size_t eax_screen_limit = eax_screen_offset + eax_name_length + eax_value_length + eax_ebx_spacer;

        const size_t ebx_screen_offset = eax_screen_limit + 1;
        const size_t ebx_screen_limit = ebx_screen_offset + ebx_name_length + ebx_value_length;

        vga_tm_strwrite(eax_screen_offset, eax_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(eax_screen_offset + eax_name_length, eax_value, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(ebx_screen_offset, ebx_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(ebx_screen_offset + ebx_name_length, ebx_value, VGA_TM_WHITE, VGA_TM_BLUE);

        // Print technical details: ECX + EDX

        const char* ecx_name = "ECX:    0x";
        char ecx_value[9];

        const char* edx_name = "EDX:    0x";
        char edx_value[9];

        itoa(state->ecx, ecx_value, 16);
        itoa(state->edx, edx_value, 16);

        const size_t ecx_edx_spacer = 8 - strlen(ecx_value);

        const size_t ecx_name_length = strlen(ecx_name);
        const size_t ecx_value_length = strlen(ecx_value);
        const size_t edx_name_length = strlen(edx_name);
        const size_t edx_value_length = strlen(edx_value);

        const size_t ecx_screen_offset = SCREEN_NEXT_LINE(80, ebx_screen_limit);
        const size_t ecx_screen_limit = ecx_screen_offset + ecx_name_length + ecx_value_length + ecx_edx_spacer;

        const size_t edx_screen_offset = ecx_screen_limit + 1;
        const size_t edx_screen_limit = edx_screen_offset + edx_name_length + edx_value_length;

        vga_tm_strwrite(ecx_screen_offset, ecx_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(ecx_screen_offset + ecx_name_length, ecx_value, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(edx_screen_offset, edx_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(edx_screen_offset + edx_name_length, edx_value, VGA_TM_WHITE, VGA_TM_BLUE);

        // Print technical details: ESI + EDI

        const char* esi_name = "ESI:    0x";
        char esi_value[9];

        const char* edi_name = "EDI:    0x";
        char edi_value[9];

        itoa(state->esi, esi_value, 16);
        itoa(state->edi, edi_value, 16);

        const size_t esi_edi_spacer = 8 - strlen(esi_value);

        const size_t esi_name_length = strlen(esi_name);
        const size_t esi_value_length = strlen(esi_value);
        const size_t edi_name_length = strlen(edi_name);
        const size_t edi_value_length = strlen(edi_value);

        const size_t esi_screen_offset = SCREEN_NEXT_LINE(80, edx_screen_limit);
        const size_t esi_screen_limit = esi_screen_offset + esi_name_length + esi_value_length + esi_edi_spacer;

        const size_t edi_screen_offset = esi_screen_limit + 1;
        const size_t edi_screen_limit = edi_screen_offset + edi_name_length + edi_value_length;

        vga_tm_strwrite(esi_screen_offset, esi_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(esi_screen_offset + esi_name_length, esi_value, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(edi_screen_offset, edi_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(edi_screen_offset + edi_name_length, edi_value, VGA_TM_WHITE, VGA_TM_BLUE);

        // Print technical details: CS + SS

        const char* cs_name = "CS:     0x";
        char cs_value[9];

        const char* ss_name = "SS:     0x";
        char ss_value[9];

        itoa(state->cs, cs_value, 16);
        itoa(state->ss, ss_value, 16);

        const size_t cs_ss_spacer = 8 - strlen(cs_value);

        const size_t cs_name_length = strlen(cs_name);
        const size_t cs_value_length = strlen(cs_value);
        const size_t ss_name_length = strlen(ss_name);
        const size_t ss_value_length = strlen(ss_value);

        const size_t cs_screen_offset = SCREEN_NEXT_LINE(80, edi_screen_limit);
        const size_t cs_screen_limit = cs_screen_offset + cs_name_length + cs_value_length + cs_ss_spacer;

        const size_t ss_screen_offset = cs_screen_limit + 1;
        const size_t ss_screen_limit = ss_screen_offset + ss_name_length + ss_value_length;

        vga_tm_strwrite(cs_screen_offset, cs_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(cs_screen_offset + cs_name_length, cs_value, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(ss_screen_offset, ss_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(ss_screen_offset + ss_name_length, ss_value, VGA_TM_WHITE, VGA_TM_BLUE);

        // Print technical details: DS

        const char* ds_name = "DS:     0x";
        char ds_value[9];

        itoa(state->ds, ds_value, 16);

        const size_t ds_name_length = strlen(ds_name);

        const size_t ds_screen_offset = SCREEN_NEXT_LINE(80, ss_screen_limit);

        vga_tm_strwrite(ds_screen_offset, ds_name, VGA_TM_WHITE, VGA_TM_BLUE);
        vga_tm_strwrite(ds_screen_offset + ds_name_length, ds_value, VGA_TM_WHITE, VGA_TM_BLUE);
    }

    // Halt the system
    while(1);
}
