#include <drivers/video/vga/textmode.h>

extern uint16_t *const vga_tm_video_memory;

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

int32_t vga_tm_write(size_t cell, uint8_t ch, uint8_t fgcolor, uint8_t bgcolor) {
    const size_t TOTAL_CELLS = vga_current_video_mode->width * vga_current_video_mode->height;

    if(cell >= TOTAL_CELLS) {
        return -1;
    }

    uint16_t entry = VGA_TM_ENTRY(ch, fgcolor, bgcolor);
    vga_tm_video_memory[cell] = entry;

    return 0;
}

int32_t vga_tm_strwrite(size_t offset, const char* str, uint8_t fgcolor, uint8_t bgcolor) {
    size_t cell = offset;

    while(*str) {
        int32_t done = vga_tm_write(cell++, *str++, fgcolor, bgcolor);

        if(done < 0) {
            return done;
        }
    }

    return 0;
}
