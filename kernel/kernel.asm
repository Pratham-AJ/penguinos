BITS 32

SECTION .text
global kernel_entry
extern kernel_main

kernel_entry:

    call kernel_main

halt:
    cli
    hlt
    jmp halt
