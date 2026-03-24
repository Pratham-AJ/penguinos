#include <stdint.h>
#include "keyboard.h"
#include "port.h"
#include "screen.h"
#include "irq.h"
#include "fb.h"
#include "calculator.h"
#include "texteditor.h"
#include "snake.h"

/* ── Scancode tables ─────────────────────────────────────────── */
#define SCANCODE_A       0x1E
#define SCANCODE_J       0x24
#define SCANCODE_A_REL   0x9E
#define SCANCODE_J_REL   0xA4

static const char scancode_map[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', 0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0
};

static const char scancode_map_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,  'A','S','D','F','G','H','J','K','L',':','"','~',
    0,  '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0, ' ', 0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0
};

static int shift_held       = 0;
static int key_a_held       = 0;
static int key_j_held       = 0;
static int extended_prefix  = 0;

/* Mode: 0=desktop 1=calculator 2=texteditor 3=snake */
static int  mode             = 0;
static int  splash_dismissed = 0;

/* ── Home screen ─────────────────────────────────────────────── */
static void draw_desktop(void)
{
    /* ── Background: vertical dark-blue gradient ── */
    for (uint32_t y = 0; y < fb.height; y++) {
        uint8_t shade = (uint8_t)(y * 18 / fb.height);
        uint32_t col = (uint32_t)((0x0A + shade) << 16) |
                       (uint32_t)((0x0F + shade) << 8)  |
                       (uint32_t)(0x1E + shade * 2);
        fb_fill_rect(0, y, fb.width, 1, col);
    }

    /* ── Big "PENGUIN" title – centred near top, scale=6 ── */
    /* Each char is 8*6+6 = 54px wide; 7 chars + 6 gaps → 7*54 = 378px */
    const char *title = "PENGUIN";
    uint32_t t_scale  = 6;
    uint32_t t_cw     = 8 * t_scale + t_scale;   /* 54 px per char */
    int      t_len    = 7;
    uint32_t t_w      = (uint32_t)t_len * t_cw;
    uint32_t t_x      = (fb.width - t_w) / 2;
    uint32_t t_y      = 28;
    /* Teal colour matching the screenshot */
    fb_draw_string_scaled(t_x, t_y, title, 0x4FC3F7, t_scale);

    /* ── Penguin logo – centred ── */
    fb_draw_logo();   /* already draws at centre of screen */

    /* ── Left-side app list panel ── */
    uint32_t panel_w  = 500;
    uint32_t panel_h  = 260;
    uint32_t px       = 12;
    uint32_t py       = (fb.height - panel_h) / 2 + 30;

    /* Semi-transparent dark overlay (solid dark blue, slightly alpha-like) */
    fb_fill_rect(px, py, panel_w, panel_h, 0x0D1220);
    /* Thin accent border only on right+bottom for the "overlay" feel */
    fb_fill_rect(px + panel_w - 1, py, 1, panel_h, 0x1E3A4A);
    fb_fill_rect(px, py + panel_h - 1, panel_w, 1, 0x1E3A4A);

    uint32_t lx   = px + 16;
    uint32_t ly   = py + 18;
    uint32_t row_h = 62;

    /* Row 1 – Calculator */
    fb_draw_string(lx,      ly,      "1. Calculator",     COLOR_YELLOW, 0x0D1220);
    fb_draw_string(lx + 18, ly + 14, "Press  C  to open", COLOR_GRAY,  0x0D1220);
    ly += row_h;

    /* Row 2 – Text Editor */
    fb_draw_string(lx,      ly,      "2. Text Editor",    COLOR_YELLOW, 0x0D1220);
    fb_draw_string(lx + 18, ly + 14, "Press  T  to open", COLOR_GRAY,  0x0D1220);
    ly += row_h;

    /* Row 3 – Snake Game */
    fb_draw_string(lx,      ly,      "3. Snake Game",     COLOR_YELLOW, 0x0D1220);
    fb_draw_string(lx + 18, ly + 14, "Press  S  to open", COLOR_GRAY,  0x0D1220);
    ly += row_h;

    /* Footer hint */
    fb_draw_string(lx, ly + 4, "Q = quit any app back to desktop",
                   0x446655, 0x0D1220);

    /* ── Taskbar ── */
    fb_fill_rect(0, fb.height - 30, fb.width, 30, 0x0A0F1A);
    fb_fill_rect(0, fb.height - 30, fb.width, 1,  0x1A3040);
    fb_draw_string(8, fb.height - 20,
                   "PenguinOS v1.13", COLOR_ACCENT, 0x0A0F1A);
    fb_draw_string(8 + 16 * 9, fb.height - 20,
                   "By Pratham, Tejas and Ayush", COLOR_WHITE, 0x0A0F1A);
}

/* ── Launchers ───────────────────────────────────────────────── */
static void launch_calculator(void)
{
    mode = 1;
    fb_clear(0x0A0F1E);
    fb_fill_rect(0, fb.height - 30, fb.width, 30, 0x0A0F1A);
    fb_fill_rect(0, fb.height - 30, fb.width, 1,  0x1A3040);
    fb_draw_string(8, fb.height - 20, "PenguinOS v1.13", COLOR_ACCENT, 0x0A0F1A);
    fb_draw_string(8 + 16 * 9, fb.height - 20,
                   "By Pratham, Tejas and Ayush", COLOR_WHITE, 0x0A0F1A);
    calculator_run();
}

static void launch_texteditor(void)
{
    mode = 2;
    fb_clear(0x0A0F1E);
    texteditor_run();
}

static void launch_snake(void)
{
    mode = 3;
    snake_run();
}

/* ── Timer IRQ (IRQ0 ~18 Hz) ─────────────────────────────────── */
static void timer_irq_handler(registers_t *regs)
{
    (void)regs;
    if (mode == 2) texteditor_tick();
    if (mode == 3) snake_tick();
}

/* ── Scancode dispatcher ─────────────────────────────────────── */
static void handle_scancode(uint8_t scancode)
{
    if (scancode == 0xE0) { extended_prefix = 1; return; }

    if (extended_prefix) {
        extended_prefix = 0;
        if (mode == 3) snake_handle_scancode(scancode);
        return;
    }

    int released = (scancode & 0x80) != 0;
    uint8_t code = scancode & 0x7F;

    if (scancode == SCANCODE_A)     key_a_held = 1;
    if (scancode == SCANCODE_A_REL) key_a_held = 0;
    if (scancode == SCANCODE_J)     key_j_held = 1;
    if (scancode == SCANCODE_J_REL) key_j_held = 0;

    if (scancode == 0x2A || scancode == 0x36) { shift_held = 1; return; }
    if (scancode == 0xAA || scancode == 0xB6) { shift_held = 0; return; }

    if (!released && key_a_held && key_j_held) {
        if (mode != 1) launch_calculator();
        return;
    }

    if (released) return;

    char key = shift_held ? scancode_map_shift[code] : scancode_map[code];
    if (!key) return;

    if (!splash_dismissed) {
        splash_dismissed = 1;
        draw_desktop();
        return;
    }

    if (mode == 1) {
        if (key == 'q' || key == 'Q') { mode = 0; draw_desktop(); }
        else calculator_handle_key(key);
        return;
    }
    if (mode == 2) {
        if (key == 'q' || key == 'Q') { mode = 0; draw_desktop(); }
        else texteditor_handle_key(key);
        return;
    }
    if (mode == 3) {
        if (key == 'q' || key == 'Q') { mode = 0; draw_desktop(); }
        return;
    }

    /* Desktop */
    if (key == 'c' || key == 'C') launch_calculator();
    else if (key == 't' || key == 'T') launch_texteditor();
    else if (key == 's' || key == 'S') launch_snake();
}

/* ── IRQ plumbing ────────────────────────────────────────────── */
static void keyboard_irq_handler(registers_t *regs)
{
    (void)regs;
    keyboard_handler();
}

void keyboard_handler(void)
{
    uint8_t scancode = inb(0x60);
    handle_scancode(scancode);
}

void keyboard_on_key(char key) { (void)key; }

void keyboard_init(void)
{
    irq_install_handler(0, timer_irq_handler);
    irq_install_handler(1, keyboard_irq_handler);
}
