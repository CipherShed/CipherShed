SECTIONS
{
    .jump 0x7c00 + 0x0000:
    {
        KEEP(*(.text.jump));
    }
    .text 0x7c00 + 0x005a :
    {
        __bootcode_start = .;
        KEEP(*(.text.startup));
        *(.text.main);
        *(.text*);
        *(.text);
    }
    .rodata :
    {
        *(.rodata*);
        *(.rdata);
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
