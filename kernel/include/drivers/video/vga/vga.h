#ifndef _KERNEL_DRIVERS_VIDEO_VGA_VGA_H
#define _KERNEL_DRIVERS_VIDEO_VGA_VGA_H

#include <system/ports.h>
#include <stdint.h>
#include <stddef.h>
#include <memory/vmm.h>
#include <stdbool.h>

#define VGA_NUM_SEQ_REGISTERS   5
#define VGA_NUM_CRTC_REGISTERS  25
#define VGA_NUM_GC_REGISTERS    9
#define VGA_NUM_AC_REGISTERS    21

// Graphics controller registers

#define VGA_GC_ADDRESS_REGISTER_PORT 0x3CE
#define VGA_GC_DATA_REGISTER_PORT 0x3CF

#define VGA_GC_SET_RESET_REGISTER 0x00
#define VGA_GC_ENABLE_SET_RESET_REGISTER 0x01
#define VGA_GC_COLOR_COMPARE_REGISTER 0x02
#define VGA_GC_DATA_ROTATE_REGISTER 0x03
#define VGA_GC_READ_MAP_SELECT_REGISTER 0x04
#define VGA_GC_MODE_REGISTER 0x05
#define VGA_GC_MISC_REGISTER 0x06
#define VGA_GC_COLOR_DONT_CARE_REGISTER 0x07
#define VGA_GC_BIT_MASK_REGISTER 0x08

// Sequencer registers

#define VGA_SEQ_ADDRESS_REGISTER_PORT 0x3C4
#define VGA_SEQ_DATA_REGISTER_PORT 0x3C5

#define VGA_SEQ_RESET_REGISTER 0x00
#define VGA_SEQ_CLOCKING_MODE_REGISTER 0x01
#define VGA_SEQ_MAP_MASK_REGISTER 0x02
#define VGA_SEQ_CHARACTER_MAP_SELECT_REGISTER 0x03
#define VGA_SEQ_MEMORY_MODE_REGISTER 0x04

// CRT controller registers

#define VGA_CRTC_ADDRESS_REGISTER_PORT 0x3D4
#define VGA_CRTC_DATA_REGISTER_PORT 0x3D5

#define VGA_CRTC_HORIZONTAL_TOTAL_REGISTER 0x00
#define VGA_CRTC_HORIZONTAL_DISPLAY_END_REGISTER 0x01
#define VGA_CRTC_START_HORIZONTAL_BLANKING_REGISTER 0x02
#define VGA_CRTC_END_HORIZONTAL_BLANKING_REGISTER 0x03
#define VGA_CRTC_START_HORIZONTAL_RETRACE_REGISTER 0x04
#define VGA_CRTC_END_HORIZONTAL_RETRACE_REGISTER 0x05
#define VGA_CRTC_VERTICAL_TOTAL_REGISTER 0x06
#define VGA_CRTC_OVERFLOW_REGISTER 0x07
#define VGA_CRTC_PRESET_ROW_SCAN_REGISTER 0x08
#define VGA_CRTC_MAXIMUM_SCAN_LINE_REGISTER 0x09
#define VGA_CRTC_CURSOR_START_REGISTER 0x0A
#define VGA_CRTC_CURSOR_END_REGISTER 0x0B
#define VGA_CRTC_START_ADDRESS_HIGH_REGISTER 0x0C
#define VGA_CRTC_START_ADDRESS_LOW_REGISTER 0x0D
#define VGA_CRTC_CURSOR_LOCATION_HIGH_REGISTER 0x0E
#define VGA_CRTC_CURSOR_LOCATION_LOW_REGISTER 0x0F
#define VGA_CRTC_VERTICAL_RETRACE_START_REGISTER 0x10
#define VGA_CRTC_VERTICAL_RETRACE_END_REGISTER 0x11
#define VGA_CRTC_VERTICAL_DISPLAY_END_REGISTER 0x12
#define VGA_CRTC_OFFSET_REGISTER 0x13
#define VGA_CRTC_UNDERLINE_LOCATION_REGISTER 0x14
#define VGA_CRTC_START_VERTICAL_BLANKING_REGISTER 0x15
#define VGA_CRTC_END_VERTICAL_BLANKING_REGISTER 0x16
#define VGA_CRTC_MODE_CONTROL_REGISTER 0x17
#define VGA_CRTC_LINE_COMPARE_REGISTER 0x18

// Attribute controller registers

#define VGA_AC_ADDRESS_REGISTER_PORT 0x3C0
#define VGA_AC_READ_REGISTER_PORT 0x3C1

#define VGA_AC_ATTRIBUTE_ADDRESS_REGISTER 0x3C0
#define VGA_AC_PALETTE_REGISTER_0 0x00
#define VGA_AC_PALETTE_REGISTER_1 0x01
#define VGA_AC_PALETTE_REGISTER_2 0x02
#define VGA_AC_PALETTE_REGISTER_3 0x03
#define VGA_AC_PALETTE_REGISTER_4 0x04
#define VGA_AC_PALETTE_REGISTER_5 0x05
#define VGA_AC_PALETTE_REGISTER_6 0x06
#define VGA_AC_PALETTE_REGISTER_7 0x07
#define VGA_AC_PALETTE_REGISTER_8 0x08
#define VGA_AC_PALETTE_REGISTER_9 0x09
#define VGA_AC_PALETTE_REGISTER_10 0x0A
#define VGA_AC_PALETTE_REGISTER_11 0x0B
#define VGA_AC_PALETTE_REGISTER_12 0x0C
#define VGA_AC_PALETTE_REGISTER_13 0x0D
#define VGA_AC_PALETTE_REGISTER_14 0x0E
#define VGA_AC_PALETTE_REGISTER_15 0x0F
#define VGA_AC_MODE_CONTROL_REGISTER 0x10
#define VGA_AC_OVERSCAN_COLOR_REGISTER 0x11
#define VGA_AC_COLOR_PLANE_ENABLE_REGISTER 0x12
#define VGA_AC_HORIZONTAL_PIXEL_PANNING_REGISTER 0x13
#define VGA_AC_COLOR_SELECT_REGISTER 0x14

// External registers

#define VGA_MISC_OUTPUT_REGISTER_READ_PORT 0x3CC
#define VGA_MISC_OUTPUT_REGISTER_WRITE_PORT 0x3C2

#define VGA_FEATURE_CONTROL_REGISTER_READ_PORT 0x3CA
#define VGA_FEATURE_CONTROL_REGISTER_MONO_WRITE_PORT 0x3BA
#define VGA_FEATURE_CONTROL_REGISTER_COLOR_WRITE_PORT 0x3DA

#define VGA_INPUT_STATUS_0_REGISTER_READ_PORT 0x3C2
#define VGA_INPUT_STATUS_1_REGISTER_MONO_READ_PORT 0x3BA
#define VGA_INPUT_STATUS_1_REGISTER_COLOR_READ_PORT 0x3DA

// Color registers

#define VGA_DAC_ADDRESS_WRITE_MODE_REGISTER 0x3C8
#define VGA_DAC_ADDRESS_READ_MODE_REGISTER 0x3C7
#define VGA_DAC_DATA_REGISTER 0x3C9
#define VGA_DAC_STATE_REGISTER 0x3C7

#define VGA_GFX_VIDEO_MEMORY    (VMM_REAL_MODE_MEMORY_BASE + 0xA0000)
#define VGA_TM_VIDEO_MEMORY     (VMM_REAL_MODE_MEMORY_BASE + 0xB8000)

#define VGA_NUM_VIDEO_MODES     20

typedef enum {
    VGA_80x25_16_TEXT = 0x03,
    VGA_640X480X16_GFX = 0x12,
    VGA_320X200X256_GFX = 0x13
} vga_video_mode_t;

struct vga_video_mode_descriptor {
    vga_video_mode_t mode;
    uint8_t *config;
    uint32_t framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t colors;
};

typedef struct vga_video_mode_descriptor vga_video_mode_descriptor_t;

/**
 * Initializes the VGA controller. In general, we have to probe for the VGA controller
 * before initializing it. However, in some cases, we can assume that the VGA controller
 * is present and skip the probing process, for example, when panicing the system.
 * 
 * @param mode The video mode to initialize.
 * @param probe Whether to probe for the VGA controller or to assume it is present.
 * @return 0 if the initialization was successful, -1 otherwise.
 */
int32_t vga_init(vga_video_mode_t mode, bool probe);

/**
 * Sets the write mode of the VGA controller.
 * 
 * @param mode The write mode to set.
 * @return 0 if the write mode was set successfully, -1 otherwise.
 */
int32_t vga_set_write_mode(uint8_t mode);

/**
 * Sets the plane mask of the VGA controller.
 * 
 * @param mask The bit mask to set.
 * @return 0 if the bit mask was set successfully, -1 otherwise.
 */
int32_t vga_set_plane_mask(uint8_t mask);

/**
 * Sets the bit mask of the VGA controller.
 * 
 * @param mask The bit mask to set.
 * @return 0 if the bit mask was set successfully, -1 otherwise.
 */
int32_t vga_set_bit_mask(uint8_t mask);

#endif // _KERNEL_DRIVERS_VIDEO_VGA_VGA_H
