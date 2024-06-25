#include <drivers/video/vga/textmode.h>

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

void vga_tm_enable_cursor(uint8_t start, uint8_t end) {
	outb(VGA_CRTC_ADDRESS_REGISTER_PORT, VGA_CRTC_CURSOR_START_REGISTER);
	outb(VGA_CRTC_DATA_REGISTER_PORT, (inb(VGA_CRTC_DATA_REGISTER_PORT) & 0xC0) | start);
 
	outb(VGA_CRTC_ADDRESS_REGISTER_PORT, VGA_CRTC_CURSOR_END_REGISTER);
	outb(VGA_CRTC_DATA_REGISTER_PORT, (inb(VGA_CRTC_DATA_REGISTER_PORT) & 0xE0) | end);
}

void vga_tm_disable_cursor() {
    outb(VGA_CRTC_ADDRESS_REGISTER_PORT, VGA_CRTC_CURSOR_START_REGISTER);
	outb(VGA_CRTC_DATA_REGISTER_PORT, 0x20);
}

int32_t vga_tm_move_cursor(size_t cell) {
	const size_t TOTAL_CELLS = vga_current_video_mode->width * vga_current_video_mode->height;

	if(cell >= TOTAL_CELLS) {
		return -1;
	}

    outb(VGA_CRTC_ADDRESS_REGISTER_PORT, VGA_CRTC_CURSOR_LOCATION_HIGH_REGISTER);
	outb(VGA_CRTC_DATA_REGISTER_PORT, cell >> 8);

	outb(VGA_CRTC_ADDRESS_REGISTER_PORT, VGA_CRTC_CURSOR_LOCATION_LOW_REGISTER);
	outb(VGA_CRTC_DATA_REGISTER_PORT, cell & 0xFF);

	return 0;
}
