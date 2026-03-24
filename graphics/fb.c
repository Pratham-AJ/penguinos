#include <stdint.h>
#include "fb.h"
#include "font.h"
#include "logo_data.h"

framebuffer_t fb;

void fb_init(uint64_t addr, uint32_t width, uint32_t height,
             uint32_t pitch, uint8_t bpp)
{
    fb.addr   = (uint8_t *)(uint32_t)addr;
    fb.width  = width;
    fb.height = height;
    fb.pitch  = pitch;
    fb.bpp    = bpp;
}

void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= fb.width || y >= fb.height) return;
   
    uint32_t bytes_per_pixel = fb.bpp / 8;
    uint8_t *pixel = (uint8_t *)fb.addr + y * fb.pitch + x * bytes_per_pixel;

    pixel[0] =  color        & 0xFF;   /* B */
    pixel[1] = (color >>  8) & 0xFF;   /* G */
    pixel[2] = (color >> 16) & 0xFF;   /* R */
    if (bytes_per_pixel == 4) pixel[3] = 0x00;
}

void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    for (uint32_t row = y; row < y + h && row < fb.height; row++)
        for (uint32_t col = x; col < x + w && col < fb.width; col++)
            fb_put_pixel(col, row, color);
}

void fb_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    fb_fill_rect(x,         y,         w, 1, color);
    fb_fill_rect(x,         y + h - 1, w, 1, color);
    fb_fill_rect(x,         y,         1, h, color);
    fb_fill_rect(x + w - 1, y,         1, h, color);
}

void fb_clear(uint32_t color)
{
    fb_fill_rect(0, 0, fb.width, fb.height, color);
}

void fb_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg)
{
    if ((unsigned char)c < 32 || (unsigned char)c > 127) c = '?';
    const unsigned char *glyph = font8x8[(unsigned char)c - 32];
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            uint32_t color = (glyph[row] & (1 << col)) ? fg : bg;
            fb_put_pixel(x + col, y + row, color);
        }
    }
}

void fb_draw_string(uint32_t x, uint32_t y, const char *s, uint32_t fg, uint32_t bg)
{
    uint32_t cx = x;
    while (*s) {
        if (*s == '\n') {
            cx = x;
            y += 10;
        } else {
            fb_draw_char(cx, y, *s, fg, bg);
            cx += 9;
        }
        s++;
    }
}


/* Draw a string with integer pixel scaling (scale=1 same as normal) */
void fb_draw_string_scaled(uint32_t x, uint32_t y, const char *s,
                           uint32_t fg, uint32_t scale)
{
    if (scale < 1) scale = 1;
    uint32_t cx = x;
    while (*s) {
        if (*s == '\n') { cx = x; y += 8 * scale + 2; s++; continue; }
        char c = *s;
        if ((unsigned char)c < 32 || (unsigned char)c > 127) c = '?';
        const unsigned char *glyph = font8x8[(unsigned char)c - 32];
        for (uint32_t row = 0; row < 8; row++) {
            for (uint32_t col = 0; col < 8; col++) {
                if (glyph[row] & (1 << col)) {
                    fb_fill_rect(cx + col * scale, y + row * scale,
                                 scale, scale, fg);
                }
            }
        }
        cx += (8 * scale) + scale;   /* glyph width + 1 scaled pixel gap */
        s++;
    }
}

void fb_draw_logo(void)
{
    uint32_t lx = (fb.width  - LOGO_WIDTH)  / 2;
    uint32_t ly = (fb.height - LOGO_HEIGHT) / 2 - 40;

    for (uint32_t row = 0; row < LOGO_HEIGHT; row++) {
        for (uint32_t col = 0; col < LOGO_WIDTH; col++) {
            uint32_t pixel = logo_pixels[row * LOGO_WIDTH + col];
            
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >>  8) & 0xFF;
            uint8_t b =  pixel        & 0xFF;
            if (r < 15 && g < 15 && b < 15) continue;
            fb_put_pixel(lx + col, ly + row, pixel);
        }
    }
}


void fb_draw_splash(void)
{
    
    for (uint32_t y = 0; y < fb.height; y++) {
        uint8_t shade = (uint8_t)(y * 20 / fb.height);
        uint32_t col = fb_color(shade, shade, shade + 30);
        fb_fill_rect(0, y, fb.width, 1, col);
    }

    
    fb_fill_rect(0, 0, fb.width, 4, COLOR_ACCENT);

    
    fb_draw_logo();

    
    uint32_t tx = (fb.width - (10 * 9)) / 2;
    fb_draw_string(tx,       fb.height / 2 + 70, "PENGUIN OS",  COLOR_WHITE,  0x00000000);
    fb_draw_string(tx + 9,   fb.height / 2 + 82, "v1.0 x86",   COLOR_ACCENT, 0x00000000);
    fb_draw_string((fb.width - (22 * 9)) / 2,
                   fb.height / 2 + 100,
                   "Press any key to continue...",
                   COLOR_GRAY, 0x00000000);

    fb_fill_rect(0, fb.height - 4, fb.width, 4, COLOR_ACCENT);
}