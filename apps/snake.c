#include <stdint.h>
#include "snake.h"
#include "fb.h"

/* ── Grid ──────────────────────────────────────────────────────── */
#define CELL         16          /* pixels per grid cell            */
#define GRID_COLS    38
#define GRID_ROWS    26
#define MAX_LEN      (GRID_COLS * GRID_ROWS)

/* Arrow-key extended scancodes (0xE0 prefix already stripped) */
#define SC_UP        0x48
#define SC_DOWN      0x50
#define SC_LEFT      0x4B
#define SC_RIGHT     0x4D

/* Tick divider: game moves every N timer IRQs (~18 Hz / 3 = 6 fps) */
#define TICKS_PER_MOVE  3

/* ── State ─────────────────────────────────────────────────────── */
typedef struct { int16_t x, y; } Cell;

static Cell     body[MAX_LEN];
static int      head;            /* ring-buffer head index          */
static int      length;
static int8_t   dx, dy;          /* current direction               */
static int8_t   ndx, ndy;        /* queued next direction           */
static Cell     food;
static int32_t  score;
static int      game_over;
static uint32_t tick_count;
static uint32_t rng_state;       /* simple LCG seed                 */
static int      started;         /* 0 = waiting for first input     */

/* ── RNG ─────────────────────────────────────────────────────── */
static uint32_t rng_next(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

/* ── Layout helpers ─────────────────────────────────────────────── */
static uint32_t grid_ox(void) { return (fb.width  - GRID_COLS * CELL) / 2; }
static uint32_t grid_oy(void) { return 40; }

static void spawn_food(void)
{
    for (int attempt = 0; attempt < 200; attempt++) {
        int fx = (int)(rng_next() % (uint32_t)GRID_COLS);
        int fy = (int)(rng_next() % (uint32_t)GRID_ROWS);
        /* Check not on snake */
        int ok = 1;
        for (int i = 0; i < length; i++) {
            int idx = (head - i + MAX_LEN) % MAX_LEN;
            if (body[idx].x == fx && body[idx].y == fy) { ok = 0; break; }
        }
        if (ok) { food.x = (int16_t)fx; food.y = (int16_t)fy; return; }
    }
    /* Fallback: just put it at 0,0 */
    food.x = 0; food.y = 0;
}

/* ── Drawing ─────────────────────────────────────────────────── */
static void int_to_str_s(int32_t v, char *buf)
{
    if (v == 0){ buf[0]='0'; buf[1]='\0'; return; }
    char tmp[12]; int i=0;
    int neg=(v<0); if(neg) v=-v;
    while(v>0){ tmp[i++]='0'+(v%10); v/=10; }
    if(neg) tmp[i++]='-';
    int j=0; while(i>0) buf[j++]=tmp[--i]; buf[j]='\0';
}

static void draw_board(void)
{
    uint32_t ox = grid_ox();
    uint32_t oy = grid_oy();
    uint32_t gw = GRID_COLS * CELL;
    uint32_t gh = GRID_ROWS * CELL;

    /* Clear desktop area */
    fb_clear(COLOR_DARKBLUE);

    /* Grid background */
    fb_fill_rect(ox, oy, gw, gh, 0x0D1117);
    fb_draw_rect(ox, oy, gw, gh, COLOR_ACCENT);

    /* Subtle grid lines */
    for (int c = 1; c < GRID_COLS; c++)
        fb_fill_rect(ox + c * CELL, oy + 1, 1, gh - 2, 0x151B22);
    for (int r = 1; r < GRID_ROWS; r++)
        fb_fill_rect(ox + 1, oy + r * CELL, gw - 2, 1, 0x151B22);

    /* Food */
    uint32_t fx = ox + food.x * CELL + 2;
    uint32_t fy = oy + food.y * CELL + 2;
    fb_fill_rect(fx,   fy,   CELL-4, CELL-4, COLOR_RED);
    fb_fill_rect(fx+2, fy+2, CELL-8, CELL-8, 0xFF6666);

    /* Snake */
    for (int i = 0; i < length; i++) {
        int idx = (head - i + MAX_LEN) % MAX_LEN;
        uint32_t sx = ox + body[idx].x * CELL + 1;
        uint32_t sy = oy + body[idx].y * CELL + 1;
        uint32_t color = (i == 0) ? COLOR_GREEN : 0x22BB44;
        uint32_t inner = (i == 0) ? 0x88FFAA : 0x117722;
        fb_fill_rect(sx,   sy,   CELL-2, CELL-2, color);
        fb_fill_rect(sx+2, sy+2, CELL-6, CELL-6, inner);
    }

    /* Status bar at bottom */
    uint32_t sb_y = oy + gh + 8;
    char sbuf[32];
    int_to_str_s(score, sbuf);

    /* "Score: " + number */
    fb_draw_string(ox, sb_y, "Score: ", COLOR_WHITE, COLOR_DARKBLUE);
    fb_draw_string(ox + 63, sb_y, sbuf, COLOR_YELLOW, COLOR_DARKBLUE);
    fb_draw_string(ox + gw - 180, sb_y,
                   "Arrows=move  Q=quit", COLOR_GRAY, COLOR_DARKBLUE);

    if (!started) {
        /* Overlay: "Press arrow to start" */
        uint32_t msg_x = ox + gw/2 - 108;
        uint32_t msg_y = oy + gh/2 - 10;
        fb_fill_rect(msg_x - 4, msg_y - 4, 228, 20, 0x1A1A2E);
        fb_draw_rect(msg_x - 4, msg_y - 4, 228, 20, COLOR_ACCENT);
        fb_draw_string(msg_x, msg_y, "Press arrow key to start!", COLOR_WHITE, 0x1A1A2E);
    }

    if (game_over) {
        uint32_t msg_x = ox + gw/2 - 100;
        uint32_t msg_y = oy + gh/2 - 16;
        fb_fill_rect(msg_x - 8, msg_y - 8, 220, 44, 0x2A0000);
        fb_draw_rect(msg_x - 8, msg_y - 8, 220, 44, COLOR_RED);
        fb_draw_string(msg_x, msg_y,      "   GAME OVER!   ", COLOR_RED,    0x2A0000);
        fb_draw_string(msg_x, msg_y + 14, "Press R to restart", COLOR_WHITE, 0x2A0000);
    }
}

/* ── Public API ─────────────────────────────────────────────────── */
void snake_run(void)
{
    rng_state  = 0xDEADBEEFu;
    head       = 0;
    length     = 4;
    dx         = 1; dy = 0;
    ndx        = 1; ndy = 0;
    score      = 0;
    game_over  = 0;
    tick_count = 0;
    started    = 0;

    /* Initial snake in middle going right */
    int sx = GRID_COLS / 2;
    int sy = GRID_ROWS / 2;
    for (int i = 0; i < length; i++) {
        body[(head - i + MAX_LEN) % MAX_LEN].x = (int16_t)(sx - i);
        body[(head - i + MAX_LEN) % MAX_LEN].y = (int16_t)sy;
    }
    spawn_food();
    draw_board();
}

void snake_handle_scancode(uint8_t sc)
{
    if (game_over) {
        if (sc == 0x13) { /* R key scancode */
            snake_run();
        }
        return;
    }

    switch (sc) {
        case SC_UP:    if (dy == 0) { ndx = 0; ndy = -1; } started = 1; break;
        case SC_DOWN:  if (dy == 0) { ndx = 0; ndy =  1; } started = 1; break;
        case SC_LEFT:  if (dx == 0) { ndx = -1; ndy = 0; } started = 1; break;
        case SC_RIGHT: if (dx == 0) { ndx =  1; ndy = 0; } started = 1; break;
        default: break;
    }
}

void snake_tick(void)
{
    if (game_over || !started) return;

    tick_count++;
    if (tick_count < TICKS_PER_MOVE) return;
    tick_count = 0;

    /* Apply queued direction */
    dx = ndx; dy = ndy;

    /* New head position */
    int new_x = body[head].x + dx;
    int new_y = body[head].y + dy;

    /* Wall collision */
    if (new_x < 0 || new_x >= GRID_COLS || new_y < 0 || new_y >= GRID_ROWS) {
        game_over = 1;
        draw_board();
        return;
    }

    /* Self collision (check all but tail tip since it will move) */
    for (int i = 0; i < length - 1; i++) {
        int idx = (head - i + MAX_LEN) % MAX_LEN;
        if (body[idx].x == new_x && body[idx].y == new_y) {
            game_over = 1;
            draw_board();
            return;
        }
    }

    /* Advance head */
    head = (head + 1) % MAX_LEN;
    body[head].x = (int16_t)new_x;
    body[head].y = (int16_t)new_y;

    /* Eat food? */
    if (new_x == food.x && new_y == food.y) {
        score += 10;
        if (length < MAX_LEN) length++;
        rng_state ^= (uint32_t)(tick_count + score);
        spawn_food();
    }

    draw_board();
}
