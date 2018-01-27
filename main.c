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
	mov $0x0e, %%ah
	int $0x10
)ASM"
			: /* no output */
			: "c"(c)
			: "al"
		);
	}
}

__attribute__((noreturn))
void main()
{
    print("Hello, World!");

    halt();
}
