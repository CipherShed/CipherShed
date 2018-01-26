OUTPUT_FORMAT(elf32-i386);
OUTPUT_ARCH(i386);
SECTIONS
{
	.text :
	{
		/*
		 * Ensure main() appears first, where main() ends up
		 * in .text.main either by section attribute, or
		 * gcc's -ffunction-sections.
		 */
		*(.text.main);
		*(.text*);
		*(.text);
	}
}
