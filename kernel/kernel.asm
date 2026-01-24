; ==================================
; kernel.asm - Low-level kernel glue
; ==================================

BITS 32

SECTION .text
    global kernel_entry
    extern kernel_main

kernel_entry:
    ; At this point:
    ; eax = multiboot magic
    ; ebx = multiboot info pointer

    push eax
    push ebx
    call kernel_main

halt:
    cli
    hlt
    jmp halt
