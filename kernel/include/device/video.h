#ifndef _KERNEL_DEVICE_VIDEO_H
#define _KERNEL_DEVICE_VIDEO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct video_driver video_driver_t;

struct video_driver {
    bool (*tm_probe)();
    bool (*gfx_probe)();

    union {
        struct {
            int32_t (*clear)();
            int32_t (*fill)(uint32_t color);
            int32_t (*write)(uint32_t cell, uint8_t ch, uint8_t fgcolor, uint8_t bgcolor);
            int32_t (*strwrite)(uint32_t offset, const char* str, uint8_t fgcolor, uint8_t bgcolor);
            int32_t (*putchar)(char ch);
            int32_t (*putstr)(const char* str);
            int32_t (*scroll)();
            int32_t (*move_cursor)(uint32_t cell);
            int32_t (*enable_cursor)(uint8_t start, uint8_t end);
            int32_t (*disable_cursor)();
            int32_t (*set_color)(uint32_t fgcolor, uint32_t bgcolor);
        } tm;

        struct {
            int32_t (*set_pixel)(uint32_t x, uint32_t y, uint32_t color);
            int32_t (*clear_screen)(uint32_t color);
            int32_t (*draw_rect)(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
            int32_t (*draw_char)(uint32_t x, uint32_t y, char c, uint32_t color);
            int32_t (*draw_string)(uint32_t x, uint32_t y, const char* str, uint32_t color);
        } gfx;
    };
};

#endif // _KERNEL_DEVICE_VIDEO_H
