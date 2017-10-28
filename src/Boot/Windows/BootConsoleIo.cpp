/*
 Copyright (c) 2008-2009 TrueCrypt Developers Association. All rights reserved.

 Governed by the TrueCrypt License 3.0 the full text of which is contained in
 the file License.txt included in TrueCrypt binary and source code distribution
 packages.
*/

#include "defaults.h"

#include "Platform.h"
#include "Bios.h"
#include "BootConsoleIo.h"
#include "BootDebug.h"
#include "BootStrings.h"


static int ScreenOutputDisabled = 0;

void DisableScreenOutput ()
{
	++ScreenOutputDisabled;
}


void EnableScreenOutput ()
{
	--ScreenOutputDisabled;
}


void PrintChar (char c)
{
#ifdef TC_BOOT_TRACING_ENABLED
	WriteDebugPort (c);
#endif

	if (ScreenOutputDisabled)
		return;

#ifndef __GNUC__
	__asm
	{
		mov bx, 7
		mov al, c
		mov ah, 0xe
		int 0x10
	}
#else
	asm volatile
	(R"ASM(
        mov $0x07, %%bx
		mov %[in], %%al
        mov $0x0e, %%ah
        int $0x10
)ASM"
		: /* no output */
		: [in] "g" (c)
		: /* no clobbers */
	);
#endif
}


void PrintCharAtCursor (char c)
{
	if (ScreenOutputDisabled)
		return;

#ifndef __GNUC__
	__asm
	{
		mov bx, 7
		mov al, c
		mov cx, 1
		mov ah, 0xa
		int 0x10
	}
#else
	asm volatile
	(R"ASM(
        mov $0x07, %%bx
		mov %[in], %%al
        mov $0x01, %%cx
        mov $0x0a, %%ah
        int $0x10
)ASM"
		: /* no output */
		: [in] "g" (c)
		: /* no clobbers */
	);
#endif
}


void Print (const char *str)
{
	char c;
	while (c = *str++)
		PrintChar (c);
}


void Print (uint32 number)
{
	char str[12];
	int pos = 0;
	while (number >= 10)
	{
		str[pos++] = (char) (number % 10) + '0';
		number /= 10;
	}
	str[pos] = (char) (number % 10) + '0';
	
	while (pos >= 0)
		PrintChar (str[pos--]);
}


void Print (const uint64 &number)
{
	if (number.HighPart == 0)
		Print (number.LowPart);
	else
		PrintHex (number);
}


void PrintHex (byte b)
{
	PrintChar (((b >> 4) >= 0xA ? 'A' - 0xA : '0') + (b >> 4));
	PrintChar (((b & 0xF) >= 0xA ? 'A' - 0xA : '0') + (b & 0xF));
}


void PrintHex (uint16 data)
{
	PrintHex (byte (data >> 8));
	PrintHex (byte (data));
}


void PrintHex (uint32 data)
{
	PrintHex (uint16 (data >> 16));
	PrintHex (uint16 (data));
}


void PrintHex (const uint64 &data)
{
	PrintHex (data.HighPart);
	PrintHex (data.LowPart);
}

void PrintRepeatedChar (char c, int n)
{
	while (n-- > 0)
		PrintChar (c);
}


void PrintEndl ()
{
	Print ("\r\n");
}


void PrintEndl (int cnt)
{
	while (cnt-- > 0)
		PrintEndl ();
}


void Beep ()
{
	PrintChar (7);
}


void InitVideoMode ()
{
	if (ScreenOutputDisabled)
		return;

#ifndef __GNUC__
	__asm
	{
		// Text mode 80x25
		mov ax, 3
		int 0x10

		// Page 0
		mov ax, 0x500
		int 0x10
	}
#else
	asm volatile
	(R"ASM(
        mov $0x03, %%ax
        int $0x10

        mov $0x0500, %%ax
        int $0x10
)ASM"
		: /* no output */
		: /* no input */
		: /* no clobbers */
	);
#endif
}


void ClearScreen ()
{
	if (ScreenOutputDisabled)
		return;

#ifndef __GNUC__
	__asm
	{
		// White text on black
		mov bh, 7
		xor cx, cx
		mov dx, 0x184f
		mov ax, 0x600
		int 0x10

		// Cursor at 0,0
		xor bh, bh
		xor dx, dx
		mov ah, 2
		int 0x10
	}
#else
	asm volatile
	(R"ASM(
        mov $0x07, %%bh
		xor %%cx, %%cx
		mov $0x184f, %%dx
		mov $0x0600, %%ax
        int $0x10

		xor %%bh, %%bh
		xor %%dx, %%dx
        mov $0x02, %%ah
        int $0x10
)ASM"
		: /* no output */
		: /* no input */
		: /* no clobbers */
	);
#endif
}


void PrintBackspace ()
{
	PrintChar (TC_BIOS_CHAR_BACKSPACE);
	PrintCharAtCursor (' ');
}


void PrintError (const char *message)
{
	Print (TC_BOOT_STR_ERROR);
	Print (message);
	PrintEndl();
	Beep();
}


void PrintErrorNoEndl (const char *message)
{
	Print (TC_BOOT_STR_ERROR);
	Print (message);
	Beep();
}


byte GetShiftFlags ()
{
	byte flags;
#ifndef __GNUC__
	__asm
	{
		mov ah, 2
		int 0x16
		mov flags, al
	}
#else
	asm volatile
	(R"ASM(
        mov 2, %%ah
        int $0x16
        mov %%al, %[out]
)ASM"
		: [out] "=g" (flags)
		: /* no input */
		: /* no clobbers */
	);
#endif

	return flags;
}


byte GetKeyboardChar ()
{
	return GetKeyboardChar (nullptr);
}


inline void yield(void)
{
#ifndef __GNUC__
	__asm
	{		
		hlt //timer interrupt will cause resume after this instruction.
	}
#else
	asm volatile
	(R"ASM(
        hlt
)ASM"
		: /* no output */
		: /* no input */
		: /* no clobbers */
	);
#endif

}


byte GetKeyboardChar (byte *scanCode)
{
	// Work around potential BIOS bugs (Windows boot manager polls the keystroke buffer)
	while (!IsKeyboardCharAvailable()) yield();

	byte asciiCode;
	byte scan;
#ifndef __GNUC__
	__asm
	{
		mov ah, 0
		int 0x16
		mov asciiCode, al
		mov scan, ah
	}
#else
	asm volatile
	(R"ASM(
        mov 0, %%ah
        int $0x16
        mov %%al, %[asciiCode]
        mov %%ah, %[scanCode]
)ASM"
		: [asciiCode] "=g" (asciiCode), [scanCode] "=g" (scan)
		: /* no input */
		: /* no clobbers */
	);
#endif
	
	if (scanCode)
		*scanCode = scan;

	return asciiCode;
}


bool IsKeyboardCharAvailable ()
{
	bool available = false;
#ifndef __GNUC__
	__asm
	{
		mov ah, 1
		int 0x16
		jz not_avail
		mov available, true
	not_avail:
	}
#else
	asm volatile
	(R"ASM(
        mov 1, %%ah
        int $0x16
		jz not_avail
        mov 1, %[out]
not_avail:
)ASM"
		: [out] "=g" (available)
		: /* no input */
		: /* no clobbers */
	);
#endif

	return available;
}


bool EscKeyPressed ()
{
	if (IsKeyboardCharAvailable ())
	{
		byte keyScanCode;
		GetKeyboardChar (&keyScanCode);
		return keyScanCode == TC_BIOS_KEY_ESC;
	}

	return false;
}


void ClearBiosKeystrokeBuffer ()
{
#ifndef __GNUC__
	__asm
	{
		push es
		xor ax, ax
		mov es, ax
		mov di, 0x41e
		mov cx, 32
		cld
		rep stosb
		pop es
	}
#else
	asm volatile
	(R"ASM(
		push es
		xor %%ax, %%ax
		mov %%ax, %%es
		mov $0x41e, %%di
		mov 32, %%cx
		cld
		rep stosb
		pop es
)ASM"
		: /* no output */
		: /* no input */
		: /* no clobbers */
	);
#endif
}


bool IsPrintable (char c)
{
	return c >= ' ' && c <= '~';
}


int GetString (char *buffer, size_t bufferSize)
{
	byte c;
	byte scanCode;
	size_t pos = 0;

	while (pos < bufferSize)
	{
		c = GetKeyboardChar (&scanCode);

		if (scanCode == TC_BIOS_KEY_ENTER)
			break;
		
		if (scanCode == TC_BIOS_KEY_ESC)
			return 0;

		buffer[pos++] = c;
		PrintChar (IsPrintable (c) ? c : ' ');
	}

	return pos;
}
