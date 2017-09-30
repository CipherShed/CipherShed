void  main() asm("main") __attribute__((stdcall));

asm (R"ASM(
	jmp main
	nop
OEMID:	.ascii "CSBOOTLD"
BPB:	.ascii "1234567890123456789012345"
EBPB:	.ascii "12345678901234567890123456"
)ASM");


__attribute__((stdcall))
inline static void print(char *string)
{
	// Clear segments
	asm volatile("mov $0, \%ax; mov \%ax, \%ds\n");

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
