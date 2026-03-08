

#include <stdint.h>

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

static uint16_t* const vga_buffer = (uint16_t*) VGA_ADDRESS;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

static void vga_put(char c, uint8_t color)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        uint16_t index = cursor_y * VGA_WIDTH + cursor_x;
        vga_buffer[index] = ((uint16_t)color << 8) | c;

        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }


    if (cursor_y >= VGA_HEIGHT) {
        cursor_y = 0;
    }
}


static void print(const char* str)
{
    while (*str) {
        vga_put(*str++, 0x0F);  
    }
}

void kernel_main(uint32_t magic, uint32_t addr)
{
    (void)magic;
    (void)addr;

    print("PENGUIN OS BOOTED SUCCESSFULLY\n");
    print("Running in 32-bit Protected Mode\n");

    while (1) {
        __asm__ volatile ("hlt");
    }
}
