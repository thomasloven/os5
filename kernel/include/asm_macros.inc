
; GRUB multiboot headers
MBOOT_PAGE_ALIGNED_FLAG	equ	1<<0
MBOOT_MEMORY_INFO_FLAG	equ	1<<1
MBOOT_HEADER_MAGIC	equ	0x1BADB002

MBOOT_HEADER_FLAGS	equ	MBOOT_PAGE_ALIGNED_FLAG | MBOOT_MEMORY_INFO_FLAG
MBOOT_HEADER_CHECKSUM	equ	-(MBOOT_HEADER_FLAGS + MBOOT_HEADER_MAGIC)


KERNEL_OFFSET	equ	0xC0000000
BOOT_STACK_SIZE	equ	0x1FFF

; SetSegments 0x10 ax loads all segment selectors with 0x10 using eax
%macro SetSegments 2
	mov e%2, %1
	mov ds, %2
	mov es, %2
	mov fs, %2
	mov gs, %2
	mov ss, %2
%endmacro

