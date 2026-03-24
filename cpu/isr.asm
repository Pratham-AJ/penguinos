
BITS 32

extern isr_handler

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0         ; dummy error code
    push byte %1        ; interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1        ; interrupt number (error code already on stack)
    jmp isr_common_stub
%endmacro

; CPU exceptions 0-31
ISR_NOERRCODE  0    ; Divide by zero
ISR_NOERRCODE  1    ; Debug
ISR_NOERRCODE  2    ; NMI
ISR_NOERRCODE  3    ; Breakpoint
ISR_NOERRCODE  4    ; Overflow
ISR_NOERRCODE  5    ; Bound range exceeded
ISR_NOERRCODE  6    ; Invalid opcode
ISR_NOERRCODE  7    ; Device not available
ISR_ERRCODE    8    ; Double fault
ISR_NOERRCODE  9    ; Coprocessor segment overrun
ISR_ERRCODE   10    ; Invalid TSS
ISR_ERRCODE   11    ; Segment not present
ISR_ERRCODE   12    ; Stack-segment fault
ISR_ERRCODE   13    ; General protection fault
ISR_ERRCODE   14    ; Page fault
ISR_NOERRCODE 15
ISR_NOERRCODE 16    ; x87 FPU error
ISR_NOERRCODE 17    ; Alignment check
ISR_NOERRCODE 18    ; Machine check
ISR_NOERRCODE 19    ; SIMD FP exception
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

isr_common_stub:
    pusha               ; Save EAX ECX EDX EBX ESP EBP ESI EDI
    mov ax, ds
    push eax            ; Save data segment

    mov ax, 0x10        ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; Pass pointer to registers struct
    call isr_handler
    add esp, 4

    pop eax             ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8          ; Remove int_no and err_code
    ; FIX: Removed sti — iret restores EFLAGS (including IF) automatically.
    ; Manually calling sti before iret on exception handlers is dangerous:
    ; it can re-enable interrupts mid-exception-return for faults that re-execute.
    iret
