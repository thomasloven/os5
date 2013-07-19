#include <elf.h>
#include <multiboot.h>
#include <vmm.h>
#include <procmm.h>
#include <memory.h>
#include <k_debug.h>
#include <vfs.h>
#include <stdlib.h>

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


void load_elf_segment(fs_node_t *file, elf_phead *phead)
{
  uint32_t flags = MM_FLAG_READ | MM_FLAG_WRITE | MM_FLAG_CANSHARE;
  uint32_t type = MM_TYPE_DATA;
  new_area(current->proc, phead->p_vaddr, phead->p_vaddr + phead->p_memsz, flags, type);

  if(phead->p_memsz == 0) return;
  vfs_read(file, phead->p_offset, phead->p_filesz, (char *)phead->p_vaddr);
  memset((void *)(phead->p_vaddr + phead->p_filesz), 0, phead->p_memsz-phead->p_filesz);
}


void load_elf(fs_node_t *file)
{
  elf_header *head = malloc(sizeof(elf_header));
  vfs_read(file, 0, sizeof(elf_header), (char *)head);
  elf_phead *program_head = malloc(sizeof(elf_phead)*head->elf_phnum);
  vfs_read(file, head->elf_phoff, sizeof(elf_phead)*head->elf_phnum, (char *)program_head);

  process_t *p = current->proc;
  process_mem_t *mm = &p->mm;

  mm->code_start = ~0x0;
  mm->code_end = 0x0;

  uint32_t i;
  for(i=0; i < head->elf_phnum; i++)
  {
    if(program_head[i].p_type == 0x1)
    {
      // If the current segment is loadable, load it and adjust
      // code area pointers accordingly.
      uintptr_t start = program_head[i].p_vaddr;
      uintptr_t end = start + program_head[i].p_memsz;
     if(start < mm->code_start)
        mm->code_start = start;
      if(end > mm->code_end)
        mm->code_end = end;

      load_elf_segment(file, &program_head[i]);
    }
  }

  mm->data_end = mm->code_end;
  mm->code_entry = head->elf_entry;
  free(program_head);
}
