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
	.bss :
	{
		*(.bss*);
		*(COMMON);
		*(.scommon);
	}
	/DISCARD/ :
	{
		*(*);
	}
}
