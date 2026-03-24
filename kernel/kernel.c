#include <stdint.h>
#include "screen.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "keyboard.h"
#include "fb.h"
#include "multiboot.h"

void kernel_main(uint32_t magic, uint32_t addr)
{
    gdt_init();
    idt_init();
    irq_init();

    /* Validate multiboot magic - if wrong, args were swapped */
    if (magic != 0x2BADB002) {
        clear_screen();
        screen_print("PENGUIN OS\n");
        screen_print("Error: bad multiboot magic\n");
        keyboard_init();
        __asm__ volatile ("sti");
        while (1) { __asm__ volatile ("hlt"); }
    }

    multiboot_info_t *mbi = (multiboot_info_t *)addr;
    int got_fb = 0;

    if (mbi &&
        (mbi->flags & MULTIBOOT_FLAG_FRAMEBUFFER) &&
        mbi->framebuffer_type == 1 &&   /* 1 = linear RGB */
        mbi->framebuffer_addr  != 0 &&
        mbi->framebuffer_width  > 0 &&
        mbi->framebuffer_height > 0)
    {
        fb_init(mbi->framebuffer_addr,
                mbi->framebuffer_width,
                mbi->framebuffer_height,
                mbi->framebuffer_pitch,
                mbi->framebuffer_bpp);
        fb_draw_splash();
        got_fb = 1;
    }

    if (!got_fb) {
        clear_screen();
        screen_print("PENGUIN OS\n");
        screen_print("VGA text mode (no framebuffer)\n");
        screen_print("Hint: run with -vga vmware or check grub.cfg\n");
    }

    keyboard_init();
    __asm__ volatile ("sti");

    while (1) {
        __asm__ volatile ("hlt");
    }
}
