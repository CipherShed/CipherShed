CROSS_PREFIX = 
CC = ${CROSS_PREFIX}gcc
AS = ${CROSS_PREFIX}as
LD = ${CROSS_PREFIX}ld
OBJCOPY = ${CROSS_PREFIX}objcopy

ARTIFACT=boot

.PHONY: all clean test

all: ${ARTIFACT}.img

${ARTIFACT}.img: ${ARTIFACT}.com
	cp $< $@
	truncate --size=4M $@

${ARTIFACT}.vmdk: ${ARTIFACT}.img
	qemu-img convert -f raw -O vmdk $< $@

main.s: main.c
	$(CC) -std=gnu11 -O0 -nostdlib -march=i386 -m16 -ffreestanding -fverbose-asm -ffunction-sections -o $@ -S $<

main.o: main.s
	$(AS) --32 -o $@ $<

startup.o: startup.s
	$(AS) --32 -o $@ $<

jump.o: jump.s
	$(AS) --32 -o $@ $<

${ARTIFACT}.o: startup.o main.o jump.o
	$(LD) -T mbr.ld -o $@ $^

${ARTIFACT}.com: ${ARTIFACT}.o
	$(OBJCOPY) $< $@ -O binary

test: ${ARTIFACT}.img
	qemu-system-i386 -nodefaults -nodefconfig -no-user-config -m 1M -device VGA -drive file=$<,format=raw -d guest_errors	

clean:
	rm -f ${ARTIFACT}.img ${ARTIFACT}.vmdk ${ARTIFACT}.com main.s *.o
