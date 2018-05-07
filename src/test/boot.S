#define ASM_FILE 1
#include "multiboot.h"
#include "gdt.h"
#include <pointer_size.h>

stack_size=0x4000
multiboot_base_flags=0

// If kernel will be linked to ELF, Multiboot will determine image load
// addresses from ELF headers, otherwise image load addresses must be
// provided in Multiboot header.
#ifdef __ELF__
	multiboot_flags=multiboot_base_flags
	multiboot_header_addr=0
	multiboot_load_addr=0
	multiboot_load_end_addr=0
	multiboot_bss_end_addr=0
	multiboot_entry_addr=0
#else
	multiboot_flags=multiboot_base_flags | MULTIBOOT_AOUT_KLUDGE
	multiboot_header_addr=multiboot_header
	multiboot_load_addr=_start
	multiboot_load_end_addr=_edata
	multiboot_bss_end_addr=_end
	multiboot_entry_addr=entry
#endif

multiboot_magic = MULTIBOOT_HEADER_MAGIC
multiboot_checksum = -(multiboot_magic + multiboot_flags)


.data
gdt:
	gdt_entry_count=0

	gdt_entry_null=gdt_entry_count
	GDT_ADD_ENTRY(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, gdt_entry_count);

	// Entries for entering real mode
	// See: Intel SDM vol. 3 9.9.2 Switching Back to Real-Address Mode

	gdt_entry_real_code=gdt_entry_count
	GDT_ADD_ENTRY(0, 10, 1, 0, 1, 0xffff, 0, 0, 0, 0, gdt_entry_count);

	gdt_entry_real_data=gdt_entry_count
	GDT_ADD_ENTRY(0, 2, 1, 0, 1, 0xffff, 0, 0, 0, 0, gdt_entry_count);

gdtr:
	.short GDT_LIMIT(gdt_entry_count)
	.long gdt

idtr:
	// See: Intel SDM vol. 3 20.1.4 Interrupt and Exception Handling
	.short 0x03ff
	.long 0x0000


// Seems like something for .data, but needs to be within first 8K of
// image.
.section .init
.balign	4, 0x00
multiboot_header:
	.long multiboot_magic
	.long multiboot_flags
	.long multiboot_checksum
	.long multiboot_header_addr
	.long multiboot_load_addr
	.long multiboot_load_end_addr
	.long multiboot_bss_end_addr
	.long multiboot_entry_addr


.code32
.text
.global entry
entry:
	lgdt (gdtr)

	// Start: switch to real mode
	// See: Intel SDM vol. 3 9.9.2 Switching Back to Real-Address Mode

	// step 1
	cli

	// step 2 unnecessary:
	// Multiboot specifies cr0.pg=0, and synced addresses

	// step 3
	ljmp $GDT_SELECT_ENTRY(gdt_entry_real_code), $code16_start
.code16
code16_start:

	// step 4
	mov $GDT_SELECT_ENTRY(gdt_entry_real_data), %ax
	mov %ax, %ss
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	// step 5
	lidt (idtr)

	// step 6
	mov %cr0, %eax
	btr $0, %eax // cr0.pe
	mov %eax, %cr0

	// step 7
	ljmp $0x00, $real_mode_start // real mode segmentation
real_mode_start:

	// step 8
	mov $0x00, %ax // see ljmp note above
	mov %ax, %ss
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	// step 9
	sti

	// End: switch to real mode

real_entry:
	// Initialize stack
	//
	// TODO: Put stack in a segment that does not overlap with
	// code segment
	//
	// However, ia16 fork of gcc currently assumes %ss == %ds (and
	// enforces this assumption).
	//
	// See: https://github.com/tkchia/gcc-ia16/issues/19
	mov $(stack+stack_size), sp_c

	jmp main

// Stack space
.comm	stack, stack_size
