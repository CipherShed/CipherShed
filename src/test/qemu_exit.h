// exit qemu with a specific exit code
//
// The exit code is : (exit_code << 1) | 1
//
// See: qemu-2.12.0's hw/misc/debug_exit.c:29
//     exit((val << 1) | 1);
// See: https://git.qemu.org/?p=qemu.git;a=blob;f=hw/misc/debugexit.c;h=84fa1a5b9d550a298283924d2710b5a65f887558;hb=4743c23509a51bd4ee85cc272287a41917d1be35#l29

//__attribute__((noreturn))
__attribute__((stdcall))
__attribute__((always_inline))
inline void qemu_exit(char exit_code)
{
	asm volatile
	(R"ASM(
	mov %[exit_code], %%al
	mov $0x501, %%dx
	outb %%al, (%%dx)
)ASM"
			: /* no output */
			: [exit_code] "r" (exit_code)
			: "al", "dx"
		);
}
