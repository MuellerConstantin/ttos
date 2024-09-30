#include <drivers/video/vga/vga.h>

uint8_t VGA_80x25_16_TEXT_CONFIG[] = {
    // External registers
	0x67, // Misc output register

    // Sequencer registers
	0x03, // Reset Register
    0x00, // Clocking Mode Register
    0x03, // Map Mask Register
    0x00, // Character Map Select Register
    0x02, // Sequencer Memory Mode Register

    // CRT controller registers
	0x5F, // Horizontal Total Register
    0x4F, // End Horizontal Display Register
    0x50, // Start Horizontal Blanking Register
    0x82, // End Horizontal Blanking Register
    0x55, // Start Horizontal Retrace Register
    0x81, // End Horizontal Retrace Register
    0xBF, // Vertical Total Register
    0x1F, // Overflow Register
	0x00, // Preset Row Scan Register
    0x4F, // Maximum Scan Line Register
    0x0D, // Cursor Start Register
    0x0E, // Cursor End Register
    0x00, // Start Address High Register
    0x00, // Start Address Low Register
    0x00, // Cursor Location High Register
    0x50, // Cursor Location Low Register
	0x9C, // Vertical Retrace Start Register
    0x0E, // Vertical Retrace End Register
    0x8F, // Vertical Display End Register
    0x28, // Offset Register
    0x1F, // Underline Location Register
    0x96, // Start Vertical Blanking Register
    0xB9, // End Vertical Blanking Register
    0xA3, // CRTC Mode Control Register
	0xFF, // Line Compare Register

    // Graphics controller registers
	0x00, // Set/Reset Register
    0x00, // Enable Set/Reset Register
    0x00, // Color Compare Register
    0x00, // Data Rotate Register
    0x00, // Read Map Select Register
    0x10, // Graphics Mode Register
    0x0E, // Miscellaneous Graphics Register
    0x00, // Color Don't Care Register
	0xFF, // Bit Mask Register

    // Attribute controller registers
	0x00,0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00
};

uint8_t VGA_320X200X256_GFX_CONFIG[] = {
    // External registers
	0x63, // Misc output register

    // Sequencer registers
	0x03, // Reset Register
    0x01, // Clocking Mode Register
    0x0F, // Map Mask Register
    0x00, // Character Map Select Register
    0x0E, // Sequencer Memory Mode Register

    // CRT controller registers
	0x5F, // Horizontal Total Register
    0x4F, // End Horizontal Display Register
    0x50, // Start Horizontal Blanking Register
    0x82, // End Horizontal Blanking Register
    0x54, // Start Horizontal Retrace Register
    0x80, // End Horizontal Retrace Register
    0xBF, // Vertical Total Register
    0x1F, // Overflow Register
	0x00, // Preset Row Scan Register
    0x41, // Maximum Scan Line Register
    0x00, // Cursor Start Register
    0x00, // Cursor End Register
    0x00, // Start Address High Register
    0x00, // Start Address Low Register
    0x00, // Cursor Location High Register
    0x00, // Cursor Location Low Register
	0x9C, // Vertical Retrace Start Register
    0x0E, // Vertical Retrace End Register
    0x8F, // Vertical Display End Register
    0x28, // Offset Register
    0x40, // Underline Location Register
    0x96, // Start Vertical Blanking Register
    0xB9, // End Vertical Blanking Register
    0xA3, // CRTC Mode Control Register
	0xFF, // Line Compare Register

    // Graphics controller registers
	0x00, // Set/Reset Register
    0x00, // Enable Set/Reset Register
    0x00, // Color Compare Register
    0x00, // Data Rotate Register
    0x00, // Read Map Select Register
    0x40, // Graphics Mode Register
    0x05, // Miscellaneous Graphics Register
    0x0F, // Color Don't Care Register
	0xFF, // Bit Mask Register

    // Attribute controller registers
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x41, 0x00, 0x0F, 0x00,	0x00
};

const vga_video_mode_descriptor_t VGA_80x25_16_TEXT_DESCRIPTOR = {
    .mode = VGA_80x25_16_TEXT,
    .config = VGA_80x25_16_TEXT_CONFIG,
    .framebuffer = VGA_TM_VIDEO_MEMORY,
    .width = 80,
    .height = 25,
    .colors = 16
};

const vga_video_mode_descriptor_t VGA_320X200X256_GFX_DESCRIPTOR = {
    .mode = VGA_320X200X256_GFX,
    .config = VGA_320X200X256_GFX_CONFIG,
    .framebuffer = VGA_GFX_VIDEO_MEMORY,
    .width = 320,
    .height = 200,
    .colors = 256
};

const vga_video_mode_descriptor_t *const VGA_VIDEO_MODE_DESCRIPTOR_TABLE[VGA_NUM_VIDEO_MODES] = {
    NULL,
    NULL,
    NULL,
    &VGA_80x25_16_TEXT_DESCRIPTOR,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &VGA_320X200X256_GFX_DESCRIPTOR
};
