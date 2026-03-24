#ifndef SNAKE_H
#define SNAKE_H

/* Call once to start/restart the game */
void snake_run(void);

/* Call from keyboard IRQ handler – accepts arrow keys via special codes */
/* Arrow key raw scancodes are passed directly for up/down/left/right     */
void snake_handle_scancode(uint8_t sc);

/* Call from the timer IRQ (IRQ0, ~18 Hz) each tick */
void snake_tick(void);

#endif
