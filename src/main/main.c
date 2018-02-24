#include "halt.h"

__attribute__((stdcall))
inline static void print(char *string)
{
	char c;
	while(c = *string++)
	{
		asm volatile
		(R"ASM(
	mov 7, %%bx
	mov %[c], %%al
	mov $0x0e, %%ah
	int $0x10
)ASM"
			: /* no output */
			: [c] "r" (c)
			: "bx", "al", "ah"
		);
	}
}

__attribute__((section(".text.main")))
__attribute__((noreturn))
void cmain() asm("main");
void cmain()
{
    print("Hello, World!");

    halt();
}
