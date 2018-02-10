ENTRY(start);
SECTIONS
{
	. = 0x100;
	.text :
	{
		*(.init);
		*(.text*);
	}
	.data :
	{
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
