void  main() asm("main") __attribute__((stdcall));

asm (R"ASM(
	.code16gcc
	jmp main
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
    asm (R"ASM(
loop:	hlt
	jmp loop
)ASM");
}
