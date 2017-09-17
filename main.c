asm (
	".code16gcc\n"
	"call main\n"
	"loop:\n"
	"hlt\n"
	"jmp loop\n"
);

inline static void print(char *string)
{
	char c;
	while(c = *string++)
	{
		asm volatile
		(
			"mov 7, %%bx\n"
			"mov $0x0e, %%ah\n"
			"int $0x10\n"
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
