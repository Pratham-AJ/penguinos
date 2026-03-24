#include <stdint.h>
#include "irq.h"
#include "idt.h"
#include "port.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

static irq_handler_t irq_handlers[16];

static void pic_remap(void)
{
    uint8_t m1 = inb(PIC1_DATA);
    uint8_t m2 = inb(PIC2_DATA);

    outb(PIC1_CMD,  0x11); io_wait();   
    outb(PIC2_CMD,  0x11); io_wait();
    outb(PIC1_DATA, 0x20); io_wait();  
    outb(PIC2_DATA, 0x28); io_wait();   
    outb(PIC1_DATA, 0x04); io_wait();   
    outb(PIC2_DATA, 0x02); io_wait();   
    outb(PIC1_DATA, 0x01); io_wait();   
    outb(PIC2_DATA, 0x01); io_wait();


    outb(PIC1_DATA, m1);
    outb(PIC2_DATA, m2);
}

void irq_init(void)
{
    pic_remap();

  
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

void irq_install_handler(uint8_t irq, irq_handler_t handler)
{
    irq_handlers[irq] = handler;
}

void irq_uninstall_handler(uint8_t irq)
{
    irq_handlers[irq] = 0;
}


void irq_handler(registers_t *regs)
{
    irq_handler_t handler = irq_handlers[regs->int_no - 32];
    if (handler) {
        handler(regs);
    }

    
    if (regs->int_no >= 40) {
        outb(PIC2_CMD, PIC_EOI);    
    }
    outb(PIC1_CMD, PIC_EOI);        
}
