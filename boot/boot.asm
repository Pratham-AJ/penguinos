; ================================
; boot.asm - Multiboot entry point
; ================================

BITS 32

SECTION .multiboot
    align 4
    dd 0x1BADB002          ; Multiboot magic number
    dd 0x00000003          ; Flags (align modules, memory info)
    dd -(0x1BADB002 + 0x00000003) ; Checksum

SECTION .text
    extern kernel_entry     ; C kernel entry
    global start

start:
    cli                    ; Disable interrupts

    mov esp, stack_top     ; Set up stack

    push eax               ; Multiboot magic number
    push ebx               ; Multiboot info structure
    call kernel_entry

hang:
    hlt
    jmp hang

SECTION .bss
    align 16
stack_bottom:
    resb 16384             ; 16 KB stack
stack_top:
