ENTRY(entry);
SECTIONS
{
	. = 0x0500;
	_start = .;
	.text :
	{
		/*. += 0x0100;*/
		/* would be .text */
		*(.init);
		*(.text*);

		/* would be .data */
		*(.rodata*);
		*(.rdata);
		*(.data*);
	} = 0xff
	_edata = .;
	.bss :
	{
		*(.bss*);
		*(COMMON);
		*(.scommon);
	}
	_end = .;
	/DISCARD/ :
	{
		*(*);
	}
}
