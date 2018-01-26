ARTIFACT=boot

.PHONY: all clean test

all: ${ARTIFACT}.img

${ARTIFACT}.img: ${ARTIFACT}.com
	cp $< $@
	printf '\x55\xaa' | dd of=$@ bs=1 seek=510
	truncate --size=4M $@

${ARTIFACT}.vmdk: ${ARTIFACT}.img
	qemu-img convert -f raw -O vmdk $< $@

main.s: main.c
	$(CC) -std=gnu11 -O0 -nostdlib -march=i386 -m16 -ffreestanding -o $@ -S $<

main.o: main.s
	$(AS) -Qy --32 -o $@ $<

${ARTIFACT}.com: main.o
	$(LD) -T bootloader.ld --nmagic -m elf_i386 -o $@ $<

test: ${ARTIFACT}.img
	qemu-system-i386 -nodefaults -nodefconfig -no-user-config -m 1M -device VGA -drive file=$<,format=raw -d guest_errors	

clean:
	rm -f ${ARTIFACT}.img ${ARTIFACT}.vmdk ${ARTIFACT}.com main.s main.o
