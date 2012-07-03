#pragma once

#define MBOOT_FLAG_MEM	0x001
#define MBOOT_FLAG_DEVICE	0x002
#define MBOOT_FLAG_CMD	0x004
#define MBOOT_FLAG_MODS	0x008
#define MBOOT_FLAG_AOUT	0x010
#define MBOOT_FLAG_ELF	0x020
#define MBOOT_FLAG_MMAP	0x040
#define MBOOT_FLAG_DRIVES	0x080
#define MBOOT_FLAG_CONFIG	0x100
#define MBOOT_FLAG_NAME	0x200
#define MBOOT_FLAG_APM	0x400
#define MBOOT_FLAG_VBE	0x800

#define MBOOT_PAGE_ALIGNED_FLAG	0x01
#define MBOOT_MEMORY_INFO_FLAG	0x02

#define MBOOT_MAGIC1	0x1BADB002
#define MBOOT_MAGIC2	0x2BADB002

#define MBOOT_MEM_FLAG_FREE	0x1


#define MBOOT_HEADER_FLAGS	(MBOOT_PAGE_ALIGNED_FLAG | MBOOT_MEMORY_INFO_FLAG)
#define MBOOT_HEADER_CHECKSUM	-(MBOOT_HEADER_FLAGS + MBOOT_MAGIC1)


#ifndef __ASSEMBLER__

typedef struct mboot_info_st
{
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t symbol_num;
	uint32_t symbol_size;
	uint32_t symbol_addr;
	uint32_t symbol_shndx;
	uint32_t mmap_length;
	uint32_t mmap_addr;
	uint32_t drives_length;
	uint32_t drives_addr;
	uint32_t config_table;
	uint32_t boot_load_name;
	uint32_t apm_table;
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint32_t vbe_mode;
	uint32_t vbe_interface_seg;
	uint32_t vbe_interface_off;
	uint32_t vbe_interface_len;
}__attribute__((packed)) mboot_info_t;

typedef struct mboot_mods_st
{
	uint32_t mod_start;
	uint32_t mod_end;
	uint8_t string[4];
	uint32_t reserved;
}__attribute__((packed)) mboot_mod_t;

typedef struct mboot_mmap_entry_st
{
	uint32_t size;
	uint32_t base_addr_lower;
	uint32_t base_addr_upper;
	uint32_t length_lower;
	uint32_t length_upper;
	uint32_t type;
}__attribute__((packed)) mboot_mmap_entry_t;

#endif
