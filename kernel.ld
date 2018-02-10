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
