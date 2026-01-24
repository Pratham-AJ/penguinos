
#include <stdint.h>
#include "keyboard.h"
#include "port.h"

static const char scancode_map[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,  ' ', 0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


void keyboard_handler(void)
{
    uint8_t scancode = inb(0x60);

   
    if (scancode & 0x80)
        return;

    char key = scancode_map[scancode];
    if (key)
        keyboard_on_key(key);
}

void keyboard_init(void)
{
    /* Nothing to do yet for basic PS/2 */
}
