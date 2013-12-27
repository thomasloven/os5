#pragma once

#include <stdint.h>
#include <multiboot.h>
#include <vfs.h>

#ifndef __ASSEMBLER__

#define ELF_ST_BIND(i) ((i)>>4)
#define ELF_ST_TYPE(i) ((i)&0xF)
#define ELF_ST_INFO(b,t) (((b)<<4)+((t)&0xF))

typedef struct
{
  uint8_t identity[16];
  uint16_t type;
  uint16_t machine;
  uint32_t version;
  uint32_t entry;
  uint32_t ph_offset;
  uint32_t sh_offset;
  uint32_t flags;
  uint16_t header_size;
  uint16_t ph_size;
  uint16_t ph_num;
  uint16_t sh_size;
  uint16_t sh_num;
  uint16_t strtab_index;
}__attribute__((packed)) elf_header;

#define ELF_TYPE_EXECUTABLE 0x2

typedef struct
{
  uint32_t type;
  uint32_t offset;
  uint32_t virtual_address;
  uint32_t physical_address;
  uint32_t file_size;
  uint32_t mem_size;
  uint32_t flags;
  uint32_t align;
}__attribute__((packed)) elf_phead;

#define ELF_PT_LOAD 0x1

#define ELF_PT_X 0x1
#define ELF_PT_R 0x2
#define ELF_PT_W 0x4

typedef struct
{
  uint32_t name;
  uint32_t type;
  uint32_t flags;
  uint32_t address;
  uint32_t offset;
  uint32_t size;
  uint32_t link;
  uint32_t info;
  uint32_t address_align;
  uint32_t entry_size;
}__attribute__((packed)) elf_section_head;

typedef struct
{
  uint32_t name;
  uint32_t value;
  uint32_t size;
  uint8_t info;
  uint8_t other;
  uint16_t shindex;
}__attribute__((packed)) elf_symbol;

typedef struct
{
  elf_symbol *symtab;
  uint32_t symtab_size;
  uint8_t *strtab;
  uint32_t strtab_size;
}__attribute__((packed)) elf_t;

elf_t kernel_elf;

void  kernel_elf_init(mboot_info_t *mboot);
char *kernel_lookup_symbol(uint32_t addr);

int load_elf(INODE file);
int is_elf(INODE file);

#endif
