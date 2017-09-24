void main() asm("main");

asm (R"ASM(
.code16gcc
	call main
loop:	hlt
	jmp loop
)ASM");

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
    print("Hello, World!");
}
