#include "halt.h"
#include "qemu_exit.h"

__attribute__((stdcall))
inline static void print(char *string)
{
	char c;
	while(c = *string++)
	{
		asm volatile
		(R"ASM(
	mov %[c], %%al
	mov $0x3f8, %%dx
	outb %%al, (%%dx)
)ASM"
			: /* no output */
			: [c] "r" (c)
			: "bx", "al", "ah", "dx"
		);
	}
}

__attribute__((section(".text.main")))
__attribute__((noreturn))
void cmain() asm("main");
void cmain()
{
    print("Hello, World!\n");

	qemu_exit(0);
    halt();
}
