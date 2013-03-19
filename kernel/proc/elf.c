#include <elf.h>
#include <multiboot.h>
#include <vmm.h>
#include <strings.h>
#include <procmm.h>
#include <memory.h>
#include <k_debug.h>

void kernel_elf_init(mboot_info_t *mboot)
{
  elf_section_head *sections = assert_higher((elf_section_head *)mboot->symbol_addr);

  uint32_t stringtab = (uint32_t)(assert_higher(sections[mboot->symbol_shndx].address));
  uint32_t i;
  for(i=0; i < mboot->symbol_num; i++)
  {
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

void load_elf_segment(elf_header *image, elf_phead *phead)
{

  uint32_t flags = MM_FLAG_READ | MM_FLAG_WRITE | MM_FLAG_CANSHARE;
  uint32_t type = MM_TYPE_DATA;
  new_area(current->proc, phead->p_vaddr, phead->p_vaddr + phead->p_memsz, flags, type);

  if(phead->p_memsz == 0) return;

  memcopy(phead->p_vaddr, ((uintptr_t)image + phead->p_offset), phead->p_filesz);
  memset(phead->p_vaddr + phead->p_filesz, 0, phead->p_memsz-phead->p_filesz);
}

void load_elf(elf_header *image)
{

  elf_phead *program_head = (elf_phead *)((uintptr_t)image + image->elf_phoff);

  process_t *p = current->proc;
  process_mem_t *mm = &p->mm;

  mm->code_start = ~0x0;
  mm->code_end = 0x0;

  uint32_t i;
  for(i=0; i < image->elf_phnum; i++)
  {
    if(program_head[i].p_type == 0x1)
    {
      uintptr_t start = program_head[i].p_vaddr;
      uintptr_t end = start + program_head[i].p_memsz;
     if(start < mm->code_start)
        mm->code_start = start;
      if(end > mm->code_end)
        mm->code_end = end;

      load_elf_segment(image, &program_head[i]);
    }
  }

  mm->data_end = mm->code_end;
  mm->code_entry = image->elf_entry;
}
