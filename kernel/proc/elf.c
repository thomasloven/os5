#include <elf.h>
#include <multiboot.h>
#include <vmm.h>
#include <strings.h>
#include <k_debug.h>

void kernel_elf_init(mboot_info_t *mboot)
{
	elf_section_head *sections = \
		assert_higher((elf_section_head *)mboot->symbol_addr);

	uintptr_t stringtab = \
		(uintptr_t)(assert_higher(sections[mboot->symbol_shndx].address));
	uintptr_t i;
	for(i=0; i < mboot->symbol_num; i++)
	{
		char *name = (char *) stringtab + sections[i].name;
		if(!strcmp(name,".strtab"))
		{
			kernel_elf.strtab = assert_higher((elf_symbol *)sections[i].address);
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
		if ((addr >= kernel_elf.symtab[i].value) && \
			(addr < (kernel_elf.symtab[i].value + kernel_elf.symtab[i].size)))
		{
			char *name = (char *)((uintptr_t)kernel_elf.strtab + \
				kernel_elf.symtab[i].name);
			return name;
		}
	}
	return 0;
}
