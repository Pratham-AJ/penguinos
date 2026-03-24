BITS 32

; ============================================================
; Multiboot header — request GRUB framebuffer (graphics mode)
; Flags: bit0=align, bit1=memmap, bit2=video mode request
; ============================================================
MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ (1 << 0) | (1 << 1) | (1 << 2)
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

SECTION .multiboot
    align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM
    ; aout fields — unused, zeroed
    dd 0, 0, 0, 0, 0
    ; Video mode request: linear framebuffer 800x600x32
    dd 0        ; mode_type: 0 = linear framebuffer
    dd 800      ; width
    dd 600      ; height
    dd 32       ; depth (bits per pixel)

SECTION .text
    extern kernel_entry
    global start

start:
    cli
    mov esp, stack_top

    mov [mb_magic], eax
    mov [mb_info],  ebx

    push dword [mb_info]
    push dword [mb_magic]

    call kernel_entry

hang:
    hlt
    jmp hang

SECTION .data
    mb_magic dd 0
    mb_info  dd 0

SECTION .bss
    align 16
stack_bottom:
    resb 32768          ; 32 KB stack
stack_top:
