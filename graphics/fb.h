#ifndef FB_H
#define FB_H

#include <stdint.h>
typedef struct {
    uint8_t  *addr;
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;      
    uint8_t   bpp;
} framebuffer_t;

extern framebuffer_t fb;


void fb_init(uint64_t addr, uint32_t width, uint32_t height,
             uint32_t pitch, uint8_t bpp);


static inline uint32_t fb_color(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}


void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void fb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void fb_clear(uint32_t color);
void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void fb_draw_string(uint32_t x, uint32_t y, const char *s, uint32_t fg, uint32_t bg);
void fb_draw_logo(void);
void fb_draw_string_scaled(uint32_t x, uint32_t y, const char *s,
                           uint32_t fg, uint32_t scale);
void fb_draw_splash(void);

#define COLOR_BLACK      0x000000
#define COLOR_WHITE      0xFFFFFF
#define COLOR_RED        0xFF3333
#define COLOR_GREEN      0x33FF66
#define COLOR_BLUE       0x3399FF
#define COLOR_YELLOW     0xFFDD33
#define COLOR_CYAN       0x33FFEE
#define COLOR_GRAY       0x888888
#define COLOR_DARKGRAY   0x333333
#define COLOR_DARKBLUE   0x0A0A2E
#define COLOR_ACCENT     0x4FC3F7

#endif 
