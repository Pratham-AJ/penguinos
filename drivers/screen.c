#include <stdint.h>
#include "screen.h"


#define VGA_ADDRESS  0xB8000
#define VGA_WIDTH    80
#define VGA_HEIGHT   25
#define DEFAULT_COLOR 0x07   

static volatile uint16_t *vga = (volatile uint16_t *)VGA_ADDRESS;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

void screen_putchar(char c, unsigned char color)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            uint16_t idx = cursor_y * VGA_WIDTH + cursor_x;
            vga[idx] = ((uint16_t)color << 8) | ' ';
        }
    } else {
        uint16_t idx = cursor_y * VGA_WIDTH + cursor_x;
        vga[idx] = ((uint16_t)color << 8) | (uint8_t)c;
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        /* Scroll up one line */
        for (uint16_t i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            vga[i] = vga[i + VGA_WIDTH];
        }
        /* Clear last line */
        for (uint16_t i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            vga[i] = ((uint16_t)DEFAULT_COLOR << 8) | ' ';
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

void screen_print(const char *str)
{
    while (*str) {
        screen_putchar(*str++, DEFAULT_COLOR);
    }
}

void clear_screen(void)
{
    for (uint16_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = ((uint16_t)DEFAULT_COLOR << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}
