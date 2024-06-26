#include <sys/panic.h>

#define SCREEN_NEXT_LINE(screen_width, screen_offset) (screen_offset + (screen_width - (screen_offset % screen_width)))
#define SCREEN_NEXT_PARAGRAPH(screen_width, screen_offset) (SCREEN_NEXT_LINE(screen_width, screen_offset) + screen_width)

const char* PANIC_TITLE = "KERNEL PANIC:";
const char* PANIC_TITLE_UNDERLINE = "=============";
const char* USER_INTRODUCTION = "An error occurred and the kernel has stopped working. " \
    "The system has been halted to prevent damage. Please restart the system. " \
    "If the error persists, please contact the system administrator.";
const char* EXCEPTION_TITLE = "EXCEPTION DETAILS:";
const char* EXCEPTION_TITLE_UNDERLINE = "------------------";
const char* TECHNICAL_TITLE = "TECHNICAL DETAILS:";
const char* TECHNICAL_TITLE_UNDERLINE = "------------------";

static size_t kpanic_strlen(const char *str);
static char *kpanic_strrev(char *str);
static char *kpanic_itoa(uint32_t n, char *buf, uint32_t base);

void kpanic(const char *msg, isr_cpu_state_t *state) {
    vga_init(VGA_80x25_16_TEXT);

    vga_tm_fill(VGA_TM_BLUE);
    vga_tm_disable_cursor();

    // Print user information and exception details

    const size_t panic_title_length = kpanic_strlen(PANIC_TITLE);
    const size_t panic_title_screen_offset = 0;
    const size_t panic_title_screen_limit = panic_title_screen_offset + panic_title_length;

    const size_t panic_title_underline_screen_offset = SCREEN_NEXT_LINE(80, panic_title_screen_limit);
    const size_t panic_title_underline_screen_limit = panic_title_underline_screen_offset + panic_title_length;

    const size_t user_introduction_length = kpanic_strlen(USER_INTRODUCTION);
    const size_t user_introduction_screen_offset = SCREEN_NEXT_PARAGRAPH(80, panic_title_underline_screen_limit);
    const size_t user_introduction_screen_limit = user_introduction_screen_offset + user_introduction_length;

    const size_t exception_title_length = kpanic_strlen(EXCEPTION_TITLE);
    const size_t exception_title_screen_offset = SCREEN_NEXT_PARAGRAPH(80, user_introduction_screen_limit);
    const size_t exception_title_screen_limit = exception_title_screen_offset + exception_title_length;

    const size_t exception_title_underline_screen_offset = SCREEN_NEXT_LINE(80, exception_title_screen_limit);
    const size_t exception_title_underline_screen_limit = exception_title_underline_screen_offset + exception_title_length;

    const size_t message_screen_length = kpanic_strlen(msg) + 2;
    const size_t message_screen_offset = SCREEN_NEXT_PARAGRAPH(80, exception_title_underline_screen_limit);
    const size_t message_screen_limit = message_screen_offset + message_screen_length;

    vga_tm_strwrite(panic_title_screen_offset, PANIC_TITLE, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(panic_title_underline_screen_offset, PANIC_TITLE_UNDERLINE, VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(user_introduction_screen_offset, USER_INTRODUCTION, VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(exception_title_screen_offset, EXCEPTION_TITLE, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(exception_title_underline_screen_offset, EXCEPTION_TITLE_UNDERLINE, VGA_TM_WHITE, VGA_TM_BLUE);

    vga_tm_strwrite(message_screen_offset, "\"", VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(message_screen_offset + 1, msg, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(message_screen_limit - 1, "\"", VGA_TM_WHITE, VGA_TM_BLUE);

    // Print technical details

    const size_t technical_title_length = kpanic_strlen(TECHNICAL_TITLE);
    const size_t technical_title_screen_offset = SCREEN_NEXT_PARAGRAPH(80, message_screen_limit);
    const size_t technical_title_screen_limit = technical_title_screen_offset + technical_title_length;

    const size_t technical_title_underline_screen_offset = SCREEN_NEXT_LINE(80, technical_title_screen_limit);
    const size_t technical_title_underline_screen_limit = technical_title_underline_screen_offset + technical_title_length;

    vga_tm_strwrite(technical_title_screen_offset, TECHNICAL_TITLE, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(technical_title_underline_screen_offset, TECHNICAL_TITLE_UNDERLINE, VGA_TM_WHITE, VGA_TM_BLUE);

    // Print technical details: ESP + EBP

    const char* esp_name = "ESP:    0x";
    char esp_value[9];

    const char* ebp_name = "EBP:    0x";
    char ebp_value[9];

    kpanic_itoa(state->esp, esp_value, 16);
    kpanic_itoa(state->ebp, ebp_value, 16);

    const size_t esp_ebp_spacer = 8 - kpanic_strlen(esp_value);

    const size_t esp_name_length = kpanic_strlen(esp_name);
    const size_t esp_value_length = kpanic_strlen(esp_value);
    const size_t ebp_name_length = kpanic_strlen(ebp_name);
    const size_t ebp_value_length = kpanic_strlen(ebp_value);

    const size_t esp_screen_offset = SCREEN_NEXT_PARAGRAPH(80, technical_title_underline_screen_limit);
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

    kpanic_itoa(state->eip, eip_value, 16);
    kpanic_itoa(state->eflags, eflags_value, 16);

    const size_t eip_eflags_spacer = 8 - kpanic_strlen(eip_value);

    const size_t eip_name_length = kpanic_strlen(eip_name);
    const size_t eip_value_length = kpanic_strlen(eip_value);
    const size_t eflags_name_length = kpanic_strlen(eflags_name);
    const size_t eflags_value_length = kpanic_strlen(eflags_value);

    const size_t eip_screen_offset = SCREEN_NEXT_LINE(80, ebp_screen_limit);
    const size_t eip_screen_limit = eip_screen_offset + eip_name_length + eip_value_length + eip_eflags_spacer;

    const size_t eflags_screen_offset = eip_screen_limit + 1;
    const size_t eflags_screen_limit = eflags_screen_offset + eflags_name_length + eflags_value_length;

    vga_tm_strwrite(eip_screen_offset, eip_name, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(eip_screen_offset + eip_name_length, eip_value, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(eflags_screen_offset, eflags_name, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(eflags_screen_offset + eflags_name_length, eflags_value, VGA_TM_WHITE, VGA_TM_BLUE);

    // Print technical details: CS + SS

    const char* cs_name = "CS:     0x";
    char cs_value[9];

    const char* ss_name = "SS:     0x";
    char ss_value[9];

    kpanic_itoa(state->cs, cs_value, 16);
    kpanic_itoa(state->ss, ss_value, 16);

    const size_t cs_ss_spacer = 8 - kpanic_strlen(cs_value);

    const size_t cs_name_length = kpanic_strlen(cs_name);
    const size_t cs_value_length = kpanic_strlen(cs_value);
    const size_t ss_name_length = kpanic_strlen(ss_name);
    const size_t ss_value_length = kpanic_strlen(ss_value);

    const size_t cs_screen_offset = SCREEN_NEXT_LINE(80, eflags_screen_limit);
    const size_t cs_screen_limit = cs_screen_offset + cs_name_length + cs_value_length + cs_ss_spacer;

    const size_t ss_screen_offset = cs_screen_limit + 1;
    const size_t ss_screen_limit = ss_screen_offset + ss_name_length + ss_value_length;

    vga_tm_strwrite(cs_screen_offset, cs_name, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(cs_screen_offset + cs_name_length, cs_value, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(ss_screen_offset, ss_name, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(ss_screen_offset + ss_name_length, ss_value, VGA_TM_WHITE, VGA_TM_BLUE);

    // Print technical details: EAX + EBX

    const char* eax_name = "EAX:    0x";
    char eax_value[9];

    const char* ebx_name = "EBX:    0x";
    char ebx_value[9];

    kpanic_itoa(state->eax, eax_value, 16);
    kpanic_itoa(state->ebx, ebx_value, 16);

    const size_t eax_ebx_spacer = 8 - kpanic_strlen(eax_value);

    const size_t eax_name_length = kpanic_strlen(eax_name);
    const size_t eax_value_length = kpanic_strlen(eax_value);
    const size_t ebx_name_length = kpanic_strlen(ebx_name);
    const size_t ebx_value_length = kpanic_strlen(ebx_value);

    const size_t eax_screen_offset = SCREEN_NEXT_LINE(80, ss_screen_limit);
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

    kpanic_itoa(state->ecx, ecx_value, 16);
    kpanic_itoa(state->edx, edx_value, 16);

    const size_t ecx_edx_spacer = 8 - kpanic_strlen(ecx_value);

    const size_t ecx_name_length = kpanic_strlen(ecx_name);
    const size_t ecx_value_length = kpanic_strlen(ecx_value);
    const size_t edx_name_length = kpanic_strlen(edx_name);
    const size_t edx_value_length = kpanic_strlen(edx_value);

    const size_t ecx_screen_offset = SCREEN_NEXT_LINE(80, ebx_screen_limit);
    const size_t ecx_screen_limit = ecx_screen_offset + ecx_name_length + ecx_value_length + ecx_edx_spacer;

    const size_t edx_screen_offset = ecx_screen_limit + 1;
    const size_t edx_screen_limit = edx_screen_offset + edx_name_length + edx_value_length;

    vga_tm_strwrite(ecx_screen_offset, ecx_name, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(ecx_screen_offset + ecx_name_length, ecx_value, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(edx_screen_offset, edx_name, VGA_TM_WHITE, VGA_TM_BLUE);
    vga_tm_strwrite(edx_screen_offset + edx_name_length, edx_value, VGA_TM_WHITE, VGA_TM_BLUE);

    // Halt the system
    while(1);
}

static size_t kpanic_strlen(const char *str) {
	size_t len = 0;

	while(str && *str != '\0') {
		++str;
		++len;
	}

	return len;
}

static char *kpanic_strrev(char *str) {
    size_t len = kpanic_strlen(str);

    char *ptr1 = str;
    char *ptr2 = str + (len - 1);

    while(ptr1 < ptr2) {
        char c = *ptr1;
        *ptr1++ = *ptr2;
        *ptr2-- = c;
    }
    
    return str;
}

static char KPANIC_ITOA_CHARS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
    'U', 'V', 'W', 'X', 'Y', 'Z'};

static char *kpanic_itoa(uint32_t n, char *buf, uint32_t base) {
    char *ptr = buf;

    if (base < 2 || base > 32) {
        *ptr = '\0';
        return buf;
    } else if (n == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return buf;
    }

    while (n != 0) {
        *ptr++ = KPANIC_ITOA_CHARS[n % base];
        n /= base;
    }

    *ptr = '\0';

    return kpanic_strrev(buf);
}
