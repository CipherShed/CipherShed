CROSS_PREFIX=
CC = ${CROSS_PREFIX}gcc

CFLAGS = -m32 -nostdlib -ffreestanding -fdata-sections -ffunction-sections -I.

all: kernel
.PHONY: all

clean:
	rm -f kernel *.o
.PHONY: clean

test: kernel
	qemu-system-i386 -nodefaults -nodefconfig -no-user-config -m 1M -device VGA -d guest_errors -kernel kernel
.PHONY: test

boot.o: boot.S
	$(CC) ${CFLAGS} -o $@ -c $<

kernel.o: kernel.c
	$(CC) ${CFLAGS} -o $@ -c $<

kernel: boot.o kernel.o
#	ld -m elf_i386 -T kernel.ld -o $@ $^
	$(CC) ${CFLAGS} -Wl,-Tkernel.ld -o $@ $^
