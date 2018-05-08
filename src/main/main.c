#include "halt.h"
#include <stdint.h>
#include <stdbool.h>
#include <mbr.h>

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
inline static void read_sector(uint8_t drive, uint16_t chs_cs, uint8_t chs_h, char* buf)
{
	asm volatile
	(R"ASM(
	mov $0, %%ax
	mov %%ax, %%es # destination segment
	mov %[buf], %%bx # destination

	mov $2, %%ah # function

	mov %[drive], %%dl # drive
	mov %[chs_cs], %%cx # (cylinder << 6) + sector
	mov %[chs_h], %%dh # head
	mov $1, %%al # sector count

	int $0x13 # fixed disk services
)ASM"
		: /* no output */
		:
		  [drive] "m" (drive),
		  [chs_cs] "m" (chs_cs),
		  [chs_h] "m" (chs_h),
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
	partition_table_entry* entries = partition_table_entries;

	bool found = false;

	for (int i = 0; i < 4; i++)
	{
		if (entries[i].status & partition_active)
		{
			read_sector(boot_drive, entries[i].start_chs_cs, entries[i].start_chs_h, buf);
			print(buf);

			found = true;
			break;
		}
	}

	if (!found)
	{
		print("fail");
	}

    halt();
}
