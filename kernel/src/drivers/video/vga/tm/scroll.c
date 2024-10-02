#include <drivers/video/vga/tm.h>

extern uint16_t *const vga_tm_video_memory;

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

void vga_tm_scroll(uint8_t fgcolor, uint8_t bgcolor) {
    for (uint16_t index = vga_current_video_mode->width; index < (vga_current_video_mode->width * vga_current_video_mode->height); index++) {
        vga_tm_video_memory[index - 80] = vga_tm_video_memory[index];
    }

    for (uint16_t index = vga_current_video_mode->width * vga_current_video_mode->height; index < (vga_current_video_mode->width * (vga_current_video_mode->height + 1)); index++) {
        vga_tm_video_memory[index - 80] = VGA_TM_ENTRY(' ', fgcolor, bgcolor);
    }
}
