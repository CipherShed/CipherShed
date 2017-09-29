.PHONY: all clean test

all: main.img

main.img: main.com
	cp $< $@
	printf '\x55\xaa' | dd of=$@ bs=1 seek=510
	truncate --size=512 $@

main.s: main.c
	$(CC) -std=gnu11 -O0 -nostdlib -march=i386 -ffreestanding -o $@ -S $<

main.o: main.s
	$(AS) -Qy --32 -o $@ $<

main.com: main.o
	$(LD) -T bootloader.ld --nmagic -m elf_i386 -o $@ $<

test: main.img
	qemu-system-i386 -nodefaults -nodefconfig -no-user-config -m 1M -device VGA -drive file=$<,format=raw -d guest_errors	

clean:
	rm -f main.img main.com main.s main.o
