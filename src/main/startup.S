#include <pointer_size.h>

.code16
.section .text.startup
startup:
	// Clear segments
	mov $0, %ax
	mov %ax, %ds

	// Setup stack
	//
	// TODO: Put stack in a segment that does not overlap with
	// code segment
	//
	// However, ia16 fork of gcc currently assumes %ss == %ds (and
	// enforces this assumption).
	//
	// See: https://github.com/tkchia/gcc-ia16/issues/19
	mov %ax, %ss
	mov $0x7c00, sp_c

	// cdecl main(boot_drive)
	pushw %dx
	call_c main
