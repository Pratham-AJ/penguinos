#include <stdint.h>
#include "texteditor.h"
#include "fb.h"
#include "keyboard.h"

/* ── Layout ─────────────────────────────────────────────────── */
#define TE_PAD_X     40
#define TE_PAD_Y     40
#define TE_TITLE_H   24
#define TE_STATUS_H  20
#define CHAR_W        9
#define CHAR_H       10
#define CURSOR_BLINK 18   /* ticks; driven by caller */

/* Buffer */
#define TE_COLS      72
#define TE_ROWS      28
#define TE_BUF_SIZE  (TE_COLS * TE_ROWS)

static char     te_buf[TE_BUF_SIZE];
static int      te_len;          /* chars currently in buffer */
static int      te_cursor;       /* index into te_buf         */
static uint32_t te_tick;         /* incremented externally    */
static int      te_cursor_vis;   /* 0 or 1                    */

/* ── Helpers ─────────────────────────────────────────────────── */
static int slen(const char *s){ int n=0; while(s[n]) n++; return n; }

static void int_to_str2(int v, char *buf)
{
    if (v == 0){ buf[0]='0'; buf[1]='\0'; return; }
    char tmp[12]; int i=0;
    while(v>0){ tmp[i++]='0'+(v%10); v/=10; }
    int j=0; while(i>0) buf[j++]=tmp[--i]; buf[j]='\0';
}

/* ── Drawing ─────────────────────────────────────────────────── */
static void te_draw(void)
{
    uint32_t win_x = TE_PAD_X;
    uint32_t win_y = TE_PAD_Y;
    uint32_t win_w = fb.width  - TE_PAD_X * 2;
    uint32_t win_h = fb.height - TE_PAD_Y * 2;

    /* Shadow */
    fb_fill_rect(win_x+4, win_y+4, win_w, win_h, 0x111111);

    /* Background */
    fb_fill_rect(win_x, win_y, win_w, win_h, 0x1A1A2E);
    fb_draw_rect(win_x, win_y, win_w, win_h, COLOR_ACCENT);

    /* Title bar */
    fb_fill_rect(win_x, win_y, win_w, TE_TITLE_H, COLOR_ACCENT);
    fb_draw_string(win_x + 8, win_y + 8, "Text Editor  [Q=quit]", 0x000000, COLOR_ACCENT);

    /* Text area background */
    uint32_t ta_x = win_x + 8;
    uint32_t ta_y = win_y + TE_TITLE_H + 6;
    uint32_t ta_w = win_w - 16;
    uint32_t ta_h = win_h - TE_TITLE_H - TE_STATUS_H - 12;
    fb_fill_rect(ta_x, ta_y, ta_w, ta_h, 0x0D0D1A);
    fb_draw_rect(ta_x, ta_y, ta_w, ta_h, 0x334455);

    /* Render characters + cursor */
    int col = 0, row = 0;
    for (int i = 0; i < te_len; i++) {
        /* Draw cursor before this char if needed */
        if (i == te_cursor && te_cursor_vis) {
            uint32_t cx = ta_x + 4 + col * CHAR_W;
            uint32_t cy = ta_y + 4 + row * CHAR_H;
            fb_fill_rect(cx, cy, 2, CHAR_H - 1, COLOR_ACCENT);
        }

        char c = te_buf[i];
        if (c == '\n') {
            col = 0; row++;
            if ((uint32_t)(row * CHAR_H) >= ta_h - CHAR_H) break;
            continue;
        }

        /* Soft-wrap */
        if (col >= TE_COLS) {
            col = 0; row++;
            if ((uint32_t)(row * CHAR_H) >= ta_h - CHAR_H) break;
        }

        uint32_t cx = ta_x + 4 + col * CHAR_W;
        uint32_t cy = ta_y + 4 + row * CHAR_H;
        fb_draw_char(cx, cy, c, COLOR_WHITE, 0x0D0D1A);
        col++;
    }

    /* Cursor at end of buffer */
    if (te_cursor == te_len && te_cursor_vis) {
        uint32_t cx = ta_x + 4 + col * CHAR_W;
        uint32_t cy = ta_y + 4 + row * CHAR_H;
        fb_fill_rect(cx, cy, 2, CHAR_H - 1, COLOR_ACCENT);
    }

    /* Status bar */
    uint32_t sb_y = win_y + win_h - TE_STATUS_H;
    fb_fill_rect(win_x + 1, sb_y, win_w - 2, TE_STATUS_H, 0x1E1E2E);
    fb_fill_rect(win_x + 1, sb_y, win_w - 2, 1, 0x334455);

    /* Compute line/col of cursor */
    int c_line = 1, c_col = 1;
    for (int i = 0; i < te_cursor; i++) {
        if (te_buf[i] == '\n') { c_line++; c_col = 1; }
        else c_col++;
    }
    char stat[48];
    /* "Ln 1  Col 1  Chars 0" */
    char nbuf[12];
    int si = 0;
    const char *lbl1 = "Ln ";
    for (int i = 0; lbl1[i]; i++) stat[si++] = lbl1[i];
    int_to_str2(c_line, nbuf);
    for (int i = 0; nbuf[i]; i++) stat[si++] = nbuf[i];
    const char *lbl2 = "  Col ";
    for (int i = 0; lbl2[i]; i++) stat[si++] = lbl2[i];
    int_to_str2(c_col, nbuf);
    for (int i = 0; nbuf[i]; i++) stat[si++] = nbuf[i];
    const char *lbl3 = "  Chars ";
    for (int i = 0; lbl3[i]; i++) stat[si++] = lbl3[i];
    int_to_str2(te_len, nbuf);
    for (int i = 0; nbuf[i]; i++) stat[si++] = nbuf[i];
    stat[si] = '\0';

    fb_draw_string(win_x + 8, sb_y + 6, stat, COLOR_GRAY, 0x1E1E2E);
}

/* ── Public API ──────────────────────────────────────────────── */
void texteditor_run(void)
{
    te_len      = 0;
    te_cursor   = 0;
    te_tick     = 0;
    te_cursor_vis = 1;
    te_draw();
}

void texteditor_handle_key(char key)
{
    /* Cursor blink: reset so it's always visible on keypress */
    te_cursor_vis = 1;
    te_tick = 0;

    if (key == '\b') {
        /* Backspace */
        if (te_cursor > 0) {
            /* Shift buffer left */
            for (int i = te_cursor - 1; i < te_len - 1; i++)
                te_buf[i] = te_buf[i + 1];
            te_len--;
            te_cursor--;
        }
    } else if (key == '\n') {
        /* Enter / newline */
        if (te_len < TE_BUF_SIZE - 1) {
            /* Shift right */
            for (int i = te_len; i > te_cursor; i--)
                te_buf[i] = te_buf[i - 1];
            te_buf[te_cursor] = '\n';
            te_len++;
            te_cursor++;
        }
    } else if (key >= 32 && key <= 126) {
        /* Printable character */
        if (te_len < TE_BUF_SIZE - 1) {
            for (int i = te_len; i > te_cursor; i--)
                te_buf[i] = te_buf[i - 1];
            te_buf[te_cursor] = key;
            te_len++;
            te_cursor++;
        }
    }

    te_draw();
}

/* Call this from the timer IRQ at ~18 Hz to blink the cursor */
void texteditor_tick(void)
{
    te_tick++;
    if (te_tick >= (uint32_t)CURSOR_BLINK) {
        te_tick = 0;
        te_cursor_vis ^= 1;
        te_draw();
    }
}
