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
  uint8_t elf_ident[16];
  uint16_t elf_type;
  uint16_t elf_machine;
  uint32_t elf_version;
  uint32_t elf_entry;
  uint32_t elf_phoff;
  uint32_t elf_shoff;
  uint32_t elf_flags;
  uint16_t elf_ehsize;
  uint16_t elf_phentsize;
  uint16_t elf_phnum;
  uint16_t elf_shentsize;
  uint16_t elf_shnum;
  uint16_t elf_shstrndx;
}__attribute__((packed)) elf_header;

typedef struct
{
  uint32_t p_type;
  uint32_t p_offset;
  uint32_t p_vaddr;
  uint32_t p_paddr;
  uint32_t p_filesz;
  uint32_t p_memsz;
  uint32_t p_flags;
  uint32_t p_align;
}__attribute__((packed)) elf_phead;

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

void load_elf(fs_node_t *file);

#endif
