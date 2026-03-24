#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

void texteditor_run(void);
void texteditor_handle_key(char key);
void texteditor_tick(void);   /* call from IRQ0 ~18 Hz for cursor blink */

#endif
