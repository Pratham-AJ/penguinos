#include "screen.h"

volatile char* video = (volatile char*)0xB8000;

void print(char *str)
{
    int i = 0;

    while(str[i])
    {
        video[i*2] = str[i];
        video[i*2+1] = 0x07;
        i++;
    }
}

void clear_screen()
{
    for(int i=0;i<80*25;i++)
    {
        video[i*2] = ' ';
        video[i*2+1] = 0x07;
    }
}
