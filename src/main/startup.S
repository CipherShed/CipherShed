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

	// boot_drive
	pushw %dx

	// At this point, the order of sections
	// (.text.startup followed by .text.main)
	// makes this equivalent to call_c'ing main
	// (with an invalid return address).
	push_c $0
