#include "halt.h"
#include <stdint.h>

__attribute__((stdcall))
inline static void print(char *string)
{
	char c;
	while(c = *string++)
	{
		asm volatile
		(R"ASM(
	mov $7, %%bx
	mov %[c], %%al
	mov $0x0e, %%ah
	int $0x10
)ASM"
			: /* no output */
			: [c] "m" (c)
			: "bx", "al", "ah"
		);
	}
}

__attribute__((stdcall))
inline static void read_sector_2(uint8_t drive, char* buf)
{
	asm volatile
	(R"ASM(
	mov $0, %%ax
	mov %%ax, %%es # destination segment
	mov %[buf], %%bx # destination

	mov $2, %%ah # function

	mov %[drive], %%dl # drive
	mov $2, %%cx # (cylinder << 6) + sector
	mov $0, %%dh # head
	mov $1, %%al # sector count

	int $0x13 # fixed disk services
)ASM"
		: /* no output */
		:
		  [drive] "m" (drive),
		  [buf] "m" (buf)
		: "ax", "bx", "cx", "dx"
	);
}

__attribute__((section(".text.main")))
__attribute__((noreturn))
void cmain(uint16_t boot_drive) asm("main");
void cmain(uint16_t boot_drive)
{
	char buf[512];

	read_sector_2(boot_drive, buf);
	print(buf);

    halt();
}
