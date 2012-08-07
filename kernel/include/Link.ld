ENTRY(start)

SECTIONS {
	. = 0xC0100000;
	.text : AT(ADDR(.text) - 0xC0000000)
	{
		code = .; _code = .; __code = .;
		*(.text)
		*(.eh_frame)
		. = ALIGN(4096);
	}
	.data : AT(ADDR(.data) - 0xC0000000)
	{
		data = .; _data = .; __data = .;
		*(.data)
		*(.rodata)
		*(.rodata.str1.1)
		. = ALIGN(4096);
	}
	.bss : AT(ADDR(.bss) - 0xC0000000)
	{
		bss = .; _bss = .; __bss = .;
		*(.bss)
		. = ALIGN(4096);
	}

	_end = .;
}
