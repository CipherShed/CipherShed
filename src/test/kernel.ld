ENTRY(entry);
SECTIONS
{
	. = 0x0500;
	_start = .;
	.text :
	{
		*(.init);
		*(.text*);
	}
	.data :
	{
		*(.rodata*);
		*(.rdata);
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
