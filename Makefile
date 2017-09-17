.PHONY: all clean test

all: main.img

main.img: main.com
	cp $< $@
	printf '\x55\xaa' | dd of=$@ bs=1 seek=510
	truncate --size=512 $@

main.com: main.c
	gcc -std=gnu99 -O0 -nostdlib -m32 -march=i386 -ffreestanding -o $@ -Wl,--nmagic,--script=bootloader.ld $<

test: main.img
	qemu-system-i386 -nodefaults -nodefconfig -no-user-config -m 1M -device VGA -drive file=$<,format=raw -d guest_errors	

clean:
	rm -f main.img main.com
