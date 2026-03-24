CC = gcc
AS = nasm
LD = ld

GCC_INCLUDE := $(shell gcc -m32 -print-file-name=include)

CFLAGS  = -m32 -ffreestanding -nostdinc -isystem $(GCC_INCLUDE) \
          -I$(CURDIR)/lib     \
          -I$(CURDIR)/include \
          -I$(CURDIR)/cpu     \
          -I$(CURDIR)/drivers \
          -I$(CURDIR)/memory  \
          -I$(CURDIR)/graphics \
          -I$(CURDIR)/apps    \
          -c -Wall -Wextra

ASFLAGS = -f elf32

OBJS = \
	boot/boot.o         \
	kernel/kernel_asm.o \
	kernel/kernel.o     \
	cpu/gdt_asm.o       \
	cpu/gdt.o           \
	cpu/idt.o           \
	cpu/isr_asm.o       \
	cpu/isr.o           \
	cpu/irq_asm.o       \
	cpu/irq.o           \
	drivers/screen.o    \
	drivers/keyboard.o  \
	graphics/fb.o       \
	graphics/font.o     \
	apps/calculator.o   \
	apps/texteditor.o   \
	apps/snake.o        \
	memory/heap.o

all: build/kernel.bin

# --- Assembly ---
boot/boot.o: boot/boot.asm
	$(AS) $(ASFLAGS) $< -o $@

kernel/kernel_asm.o: kernel/kernel.asm
	$(AS) $(ASFLAGS) $< -o $@

cpu/gdt_asm.o: cpu/gdt.asm
	$(AS) $(ASFLAGS) $< -o $@

cpu/isr_asm.o: cpu/isr.asm
	$(AS) $(ASFLAGS) $< -o $@

cpu/irq_asm.o: cpu/irq.asm
	$(AS) $(ASFLAGS) $< -o $@

# --- C sources ---
kernel/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) $< -o $@

cpu/gdt.o: cpu/gdt.c
	$(CC) $(CFLAGS) $< -o $@

cpu/idt.o: cpu/idt.c
	$(CC) $(CFLAGS) $< -o $@

cpu/isr.o: cpu/isr.c
	$(CC) $(CFLAGS) $< -o $@

cpu/irq.o: cpu/irq.c
	$(CC) $(CFLAGS) $< -o $@

drivers/screen.o: drivers/screen.c
	$(CC) $(CFLAGS) $< -o $@

drivers/keyboard.o: drivers/keyboard.c
	$(CC) $(CFLAGS) $< -o $@

graphics/fb.o: graphics/fb.c
	$(CC) $(CFLAGS) $< -o $@

graphics/font.o: graphics/font.c
	$(CC) $(CFLAGS) $< -o $@

apps/calculator.o: apps/calculator.c
	$(CC) $(CFLAGS) $< -o $@

apps/texteditor.o: apps/texteditor.c
	$(CC) $(CFLAGS) $< -o $@

apps/snake.o: apps/snake.c
	$(CC) $(CFLAGS) $< -o $@

memory/heap.o: memory/heap.c
	$(CC) $(CFLAGS) $< -o $@

# --- Link ---
build/kernel.bin: $(OBJS)
	$(LD) -m elf_i386 -T kernel/linker.ld -o build/kernel.bin $(OBJS)

# --- ISO ---
iso: build/kernel.bin
	mkdir -p iso/boot/grub
	cp build/kernel.bin iso/boot/kernel.bin
	cp boot/grub/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o build/os.iso iso

clean:
	rm -f $(OBJS) build/kernel.bin build/os.iso
