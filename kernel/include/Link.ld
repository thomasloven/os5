ENTRY(start)

SECTIONS {
  . = 0xC0100000;
  .text : AT(ADDR(.text) - 0xC0000000)
  {
    code = .;
    *(.text)
    . = ALIGN(4096);
  }
  .rodata : AT(ADDR(.rodata) - 0xC0000000)
  {
    rodata = .;
    *(.rodata*)
    . = ALIGN(4096);
  }
  .data : AT(ADDR(.data) - 0xC0000000)
  {
    data = .;
    *(.data)
    . = ALIGN(4096);
  }
  .eh_frame : AT(ADDR(.eh_frame) - 0xC0000000)
  {
    eh_frame = .;
    *(.eh_frame)
    . = ALIGN(4096);
  }
  .bss : AT(ADDR(.bss) - 0xC0000000)
  {
    bss = .;
    *(.bss)
    . = ALIGN(4096);
  }

  _end = .;
}
