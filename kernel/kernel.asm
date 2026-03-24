; ================================
; kernel.asm - Kernel ASM entry
; ================================
BITS 32

SECTION .text
global kernel_entry
extern kernel_main

; FIX: kernel_entry receives two args on the stack (magic, addr)
; from boot.asm — forward them directly to kernel_main
kernel_entry:
    ; boot.asm pushed: push [mb_info] then push [mb_magic]
    ; So stack layout when we arrive here (after CALL pushed ret addr):
    ;   [esp+0] = return address
    ;   [esp+4] = magic   (pushed last by boot.asm = top of stack)
    ;   [esp+8] = addr    (pushed first by boot.asm)
    ;
    ; kernel_main(magic, addr) needs magic at [esp+4], addr at [esp+8]
    ; relative to kernel_main's own frame — so we just re-push in correct order.
    mov eax, [esp+8]   ; eax = addr  (pushed first = deeper in stack)
    mov ecx, [esp+4]   ; ecx = magic (pushed last  = closer to top)
    push eax            ; arg2: addr
    push ecx            ; arg1: magic  ← kernel_main sees this at its [esp+4]
    call kernel_main
    add esp, 8

halt:
    cli
    hlt
    jmp halt
