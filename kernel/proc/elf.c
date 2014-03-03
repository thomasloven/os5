#include <elf.h>
#include <multiboot.h>
#include <vmm.h>
#include <procmm.h>
#include <k_debug.h>
#include <vfs.h>
#include <stdlib.h>
#include <string.h>

void kernel_elf_init(mboot_info_t *mboot)
{
  // Load data about the kernel executable for use in debugging.

  elf_section_head *sections = assert_higher((elf_section_head *)mboot->symbol_addr);

  uint32_t stringtab = (uint32_t)(assert_higher(sections[mboot->symbol_shndx].address));
  uint32_t i;
  for(i=0; i < mboot->symbol_num; i++)
  {
    // We want to find the string and symbol tables.
    char *name = (char *) stringtab + sections[i].name;
    if(!strcmp(name,".strtab"))
    {
      kernel_elf.strtab = assert_higher((uint8_t *)sections[i].address);
      kernel_elf.strtab_size = sections[i].size;
    }
    if(!strcmp(name,".symtab"))
    {
      kernel_elf.symtab = assert_higher((elf_symbol *)sections[i].address);
      kernel_elf.symtab_size = sections[i].size;
    }
  }
}

char *kernel_lookup_symbol(uint32_t addr)
{
  // Finds the name of the kernel function at addr.
  // Requires kernel_elf_init to be run first.

  uint32_t i;

  for(i=0; i < (kernel_elf.symtab_size/sizeof(elf_symbol)); i++)
  {
    if (ELF_ST_TYPE(kernel_elf.symtab[i].info) != 0x2)
      continue;
    if ((addr >= kernel_elf.symtab[i].value) && (addr < (kernel_elf.symtab[i].value + kernel_elf.symtab[i].size)))
    {
      char *name = (char *)((uint32_t)kernel_elf.strtab + kernel_elf.symtab[i].name);
      return name;
    }
  }
  return 0;
}


void load_elf_segment(INODE file, elf_phead *phead)
{
  uint32_t flags = MM_FLAG_READ;
  if(phead->flags & ELF_PT_W) flags |= MM_FLAG_WRITE;
  uint32_t type = MM_TYPE_DATA;

  uint32_t memsize = phead->mem_size; // Size in memory
  uint32_t filesize = phead->file_size; // Size in file
  uint32_t mempos = phead->virtual_address; // Offset in memory
  uint32_t filepos = phead->offset; // Offset in file

  new_area(current->proc, mempos, mempos + memsize, flags, type);

  if(memsize == 0) return; // Nothing to load

  // Read the data right into memory
  vfs_read(file, (char *)mempos, filesize, filepos);
  // Fill the rest of the area with zeros
  memset((void *)(mempos + filesize), 0, memsize-filesize);
}

int _is_elf(elf_header *elf)
{
  int iself = -1;

  if((elf->identity[0] == 0x7f) && !strncmp((char *)&elf->identity[1], "ELF", 3))
    iself = 0;

  if(iself != -1)
    iself = elf->type;

  return iself;
}
int is_elf(INODE file)
{
  elf_header *elf = malloc(sizeof(elf_header));
  vfs_read(file, (char *)elf, sizeof(elf_header), 0);

  int iself = _is_elf(elf);

  free(elf);
  return iself;
}

int load_elf(INODE file)
{
  // Read elf header from the file
  elf_header *elf = malloc(sizeof(elf_header));
  vfs_read(file, (char *)elf, sizeof(elf_header), 0);

  // Check if the file is actually an elf executable
  if(_is_elf(elf) != ELF_TYPE_EXECUTABLE)
    return -1;

  // Read program headers from the file
  elf_phead *phead = malloc(sizeof(elf_phead)*elf->ph_num);
  vfs_read(file, (char *)phead, sizeof(elf_phead)*elf->ph_num, elf->ph_offset);

  // Prepare the memory manager
  process_mem_t *mm = &current->proc->mm;
  mm->code_start = ~0x0;
  mm->code_end = 0x0;

  uint32_t i;
  for(i=0; i < elf->ph_num; i++)
  {
    if(phead[i].type == ELF_PT_LOAD)
    {
      // If the current segment is loadable, load it and adjust
      // code area pointers accordingly.
      uintptr_t start = phead[i].virtual_address;
      uintptr_t end = start + phead[i].mem_size;
     if(start < mm->code_start)
        mm->code_start = start;
      if(end > mm->code_end)
        mm->code_end = end;

      load_elf_segment(file, &phead[i]);
    }
  }

  mm->data_end = mm->code_end;
  mm->code_entry = elf->entry;
  free(phead);
  free(elf);
  return 0;
}

