OUTPUT_FORMAT(elf32-i386);
OUTPUT_ARCH(i386);
SECTIONS
{
    .jump 0x7c00 + 0x0000:
    {
        *(.text.jump);
    }
    .text 0x7c00 + 0x005a :
    {
        __bootcode_start = .;
        *(.text.startup);
        *(.text.main);
        *(.text*);
        *(.text);
    }
    .rodata :
    {
        *(.rodata*);
    }
    .data :
    {
        *(.data*);
    }
    .eos 0x7c00 + 0x01fe :
    {
        /* End of sector marker */
        SHORT(0xaa55);
    }
    /DISCARD/ : {*(*)}
}
