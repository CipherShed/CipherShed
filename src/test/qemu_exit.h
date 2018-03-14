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
