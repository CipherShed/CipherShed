ENTRY(start);
SECTIONS
{
	. = 0x100;
	.text :
	{
		/* would be .text */
		*(.init);
		*(.text*);

		/* would be .data */
		*(.rodata*);
		*(.rdata*);
		*(.data*);
	}
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
