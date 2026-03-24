#include <stdint.h>
#include "idt.h"
#include "isr.h"


static struct idt_entry idt[256];
static struct idt_ptr   idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags)
{
    idt[num].base_low  = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = selector;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}

void idt_init(void)
{
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;


    uint8_t *ptr = (uint8_t *)&idt;
    for (uint32_t i = 0; i < sizeof(idt); i++) {
        ptr[i] = 0;
    }

    
    isr_install();

    
    __asm__ volatile ("lidt (%0)" : : "r"(&idtp));
}
