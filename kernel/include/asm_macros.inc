
; GRUB multiboot headers
#define MBOOT_PAGE_ALIGNED_FLAG	(1<<0)
#define MBOOT_MEMORY_INFO_FLAG	(1<<1)
#define MBOOT_HEADER_MAGIC	(0x1BADB002)

#define MBOOT_HEADER_FLAGS	(MBOOT_PAGE_ALIGNED_FLAG | MBOOT_MEMORY_INFO_FLAG)
#define MBOOT_HEADER_CHECKSUM	-(MBOOT_HEADER_FLAGS + MBOOT_HEADER_MAGIC)

#define KERNEL_OFFSET	0xC0000000
#define BOOT_STACK_SIZE	0x1FFF

#ifdef __ASSEMBLER__
	; SetSegments 0x10 ax loads all segment selectors with 0x10 using eax
	%macro SetSegments 2
		mov e%2, %1
		mov ds, %2
		mov es, %2
		mov fs, %2
		mov gs, %2
		mov ss, %2
	%endmacro
#endif
