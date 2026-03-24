#include <stdint.h>
#include "calculator.h"
#include "fb.h"
#include "keyboard.h"

#define CALC_X       250
#define CALC_Y       150
#define CALC_W       300
#define CALC_H       300
#define DISPLAY_H    50
#define BTN_W        60
#define BTN_H        50
#define BTN_PAD      5

static void int_to_str(int32_t val, char *buf)
{
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[12];
    int i = 0;
    int neg = (val < 0);
    if (neg) val = -val;
    while (val > 0) { tmp[i++] = '0' + (val % 10); val /= 10; }
    if (neg) tmp[i++] = '-';
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
}


static int slen(const char *s) { int n=0; while(s[n]) n++; return n; }


static int32_t  num_a    = 0;
static int32_t  num_b    = 0;
static char     op       = 0;
static int      entering = 0;  
static int      fresh    = 1;   
static char     display[24];
static char     history[24];

static void calc_clear(void)
{
    num_a = num_b = 0;
    op = 0;
    entering = 0;
    fresh = 1;
    display[0] = '0'; display[1] = '\0';
    history[0] = '\0';
}

static void draw_calculator(void)
{
    uint32_t x = CALC_X, y = CALC_Y;

    
    fb_fill_rect(x+4, y+4, CALC_W, CALC_H, 0x111111);

    
    fb_fill_rect(x, y, CALC_W, CALC_H, 0x1E1E2E);
    fb_draw_rect(x, y, CALC_W, CALC_H, COLOR_ACCENT);

    
    fb_fill_rect(x, y, CALC_W, 24, COLOR_ACCENT);
    fb_draw_string(x + 8, y + 8, "Calculator  [Q=quit]", 0x000000, COLOR_ACCENT);

    
    fb_fill_rect(x+1, y+25, CALC_W-2, 14, 0x16161E);
    if (history[0]) {
        int hx = x + CALC_W - slen(history)*9 - 8;
        fb_draw_string(hx, y + 28, history, COLOR_GRAY, 0x16161E);
    }

   
    fb_fill_rect(x+1, y+40, CALC_W-2, DISPLAY_H, 0x12121A);
    fb_draw_rect(x+2, y+41, CALC_W-4, DISPLAY_H-2, 0x334455);
    int dx = x + CALC_W - slen(display)*9 - 10;
    if (dx < (int32_t)(x+6)) dx = x+6;
    fb_draw_string(dx, y + 56, display, COLOR_WHITE, 0x12121A);

    
    if (op && entering == 1) {
        char op_str[2] = {op, '\0'};
        fb_draw_string(x + 8, y + 58, op_str, COLOR_YELLOW, 0x12121A);
    }

    
    const char *labels[4][4] = {
        {"7","8","9","/"},
        {"4","5","6","*"},
        {"1","2","3","-"},
        {"C","0","=","+"}
    };
    uint32_t btn_colors[4][4] = {
        {0x2A2A3E,0x2A2A3E,0x2A2A3E,0x1A3A4A},
        {0x2A2A3E,0x2A2A3E,0x2A2A3E,0x1A3A4A},
        {0x2A2A3E,0x2A2A3E,0x2A2A3E,0x1A3A4A},
        {0x3A1A1A,0x2A2A3E,0x1A4A2A,0x1A3A4A}
    };

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            uint32_t bx = x + BTN_PAD + col * (BTN_W + BTN_PAD);
            uint32_t by = y + 96 + row * (BTN_H + BTN_PAD);
            fb_fill_rect(bx, by, BTN_W, BTN_H, btn_colors[row][col]);
            fb_draw_rect(bx, by, BTN_W, BTN_H, 0x445566);
            int lw = slen(labels[row][col]) * 9;
            fb_draw_string(bx + (BTN_W-lw)/2, by + (BTN_H-8)/2,
                           labels[row][col], COLOR_WHITE, btn_colors[row][col]);
        }
    }
}

static void calc_digit(char d)
{
    int32_t *num = (entering == 0) ? &num_a : &num_b;
    if (fresh) { *num = 0; fresh = 0; }
    if (*num < 100000000 && *num > -100000000) {
        *num = *num * 10 + (d - '0');
    }
    int_to_str(*num, display);
}

static void calc_set_op(char o)
{
    op = o;
    entering = 1;
    fresh = 1;
    int_to_str(num_a, history);
    int n = slen(history);
    history[n] = ' '; history[n+1] = op; history[n+2] = '\0';
}

static void calc_equals(void)
{
    if (!op) return;
    int32_t result = 0;
    char err = 0;
    switch (op) {
        case '+': result = num_a + num_b; break;
        case '-': result = num_a - num_b; break;
        case '*': result = num_a * num_b; break;
        case '/':
            if (num_b == 0) { err = 1; break; }
            result = num_a / num_b;
            break;
    }
    char ta[12], tb[12], tr[12];
    int_to_str(num_a, ta);
    int_to_str(num_b, tb);
    int n = 0;
    for (int i = 0; ta[i]; i++) history[n++] = ta[i];
    history[n++] = op;
    for (int i = 0; tb[i]; i++) history[n++] = tb[i];
    history[n++] = '=';
    history[n] = '\0';

    if (err) {
        display[0]='E'; display[1]='r'; display[2]='r'; display[3]='\0';
    } else {
        int_to_str(result, tr);
        for (int i = 0; tr[i]; i++) history[n++] = tr[i];
        history[n] = '\0';
        num_a = result;
        int_to_str(result, display);
    }
    op = 0; entering = 0; fresh = 1;
}

static void calc_backspace(void)
{
    int32_t *num = (entering == 0) ? &num_a : &num_b;
    *num /= 10;
    int_to_str(*num, display);
}

void calculator_handle_key(char key)
{
    if (key >= '0' && key <= '9') { calc_digit(key); }
    else if (key == '+') { calc_set_op('+'); }
    else if (key == '-') { calc_set_op('-'); }
    else if (key == '*') { calc_set_op('*'); }
    else if (key == '/') { calc_set_op('/'); }
    else if (key == '\n') { calc_equals(); }
    else if (key == '\b') { calc_backspace(); }
    else if (key == 'c' || key == 'C') { calc_clear(); }

    draw_calculator();
}

void calculator_run(void)
{
    calc_clear();
    draw_calculator();
}
