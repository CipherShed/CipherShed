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

void main()
{
    // Clear segments
    asm volatile("mov $0, \%ax; mov \%ax, \%ds\n");

    print("Hello, World!");

    halt();
}
