CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -ffreestanding -c
ASFLAGS = -f elf32

OBJS = \
boot/boot.o \
kernel/kernel.o \
kernel/kernel_asm.o \
cpu/gdt.o \
drivers/keyboard.o

all:
	$(AS) $(ASFLAGS) boot/boot.asm -o boot/boot.o
	$(AS) $(ASFLAGS) kernel/kernel.asm -o kernel/kernel_asm.o
	$(CC) $(CFLAGS) kernel/kernel.c -o kernel/kernel.o
	$(CC) $(CFLAGS) cpu/gdt.c -o cpu/gdt.o
	$(CC) $(CFLAGS) drivers/keyboard.c -o drivers/keyboard.o

	$(LD) -m elf_i386 -T kernel/linker.ld -o kernel.bin $(OBJS)

clean:
	rm -rf *.o *.bin
