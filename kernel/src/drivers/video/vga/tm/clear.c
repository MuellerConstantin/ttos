#include <drivers/video/vga/textmode.h>

extern uint16_t *const vga_tm_video_memory;

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

extern vga_tm_screen_t vga_tm_screen;

void vga_tm_clear() {
    uint16_t entry = VGA_TM_ENTRY(0x00, vga_tm_screen.fgcolor, vga_tm_screen.bgcolor);

    const size_t TOTAL_CELLS = vga_current_video_mode->width * vga_current_video_mode->height;

    for(size_t cell = 0; cell < TOTAL_CELLS; ++cell) {
        vga_tm_video_memory[cell] = entry;
    }
}

void vga_tm_fill(uint8_t bgcolor) {
    uint16_t entry = VGA_TM_ENTRY(0x00, VGA_TM_WHITE, bgcolor);

    const size_t TOTAL_CELLS = vga_current_video_mode->width * vga_current_video_mode->height;

    for(size_t cell = 0; cell < TOTAL_CELLS; ++cell) {
        vga_tm_video_memory[cell] = entry;
    }
}
