#include <elf.h>
#include <multiboot.h>
#include <vmm.h>
#include <strings.h>
#include <k_debug.h>
#include <memory.h>

void kernel_elf_init(mboot_info_t *mboot)
{
	// Load elf data from multiboot
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
	// Lookup address in the symbol table of the kernel
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

void load_elf_segment(elf_header *image, elf_phead *phead)
{
	// Load elf program segment into memory
	uint8_t *img = (uint8_t *)image;
	uint32_t first_page = ((phead->p_vaddr) & PAGE_MASK);
	uint32_t num_pages = ((phead->p_memsz + PAGE_SIZE) & PAGE_MASK)/PAGE_SIZE;
	if(num_pages == 0) return;
	uint32_t i;
	for(i = 0; i < num_pages; i++)
	{
		vmm_page_set(first_page + i*PAGE_SIZE, \
			vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_WRITE | PAGE_USER));
	}
	memcopy(phead->p_vaddr, &img[phead->p_offset], phead->p_filesz);
	memset(phead->p_vaddr + phead->p_filesz, 0, phead->p_memsz - phead->p_filesz);
}

void load_elf(elf_header *image, elf_t *elf)
{
	// Load an elf program into memory
	elf_phead *program_head = (elf_phead *)((uintptr_t)image + image->elf_phoff);
	uint32_t i;
	for(i = 0; i < image->elf_phnum; i++)
	{
		if(program_head[i].p_type == 0x1)
		{
			load_elf_segment(image, &program_head[i]);
		}
	}

	// Load aditional data into elf data structure
	elf_section_head *sections = (elf_section_head *)((uintptr_t)image + image->elf_shoff);

	uintptr_t stringtab = (uintptr_t)((uintptr_t)image + sections[image->elf_shstrndx].offset);

	for(i=0; i < image->elf_shnum; i++)
	{
		char *name = (char *) stringtab + sections[i].name;
		if(!strcmp(name,".strtab"))
		{
			if(sections[i].address)
				elf->strtab = (elf_symbol *)sections[i].address;
			else
				elf->strtab = (elf_symbol *)((uintptr_t)image  + sections[i].offset);
			elf->strtab_size = sections[i].size;
		}
		if(!strcmp(name,".symtab"))
		{
			if(sections[i].address)
				elf->symtab = (elf_symbol *)sections[i].address;
			else
				elf->symtab = (elf_symbol *)((uintptr_t)image  + sections[i].offset);
			elf->symtab_size = sections[i].size;
		}
	}

	elf->entry = image->elf_entry;
}

char *elf_lookup_symbol(elf_t *elf, uint32_t addr)
{
	// Look up an address in an elf symbol table
	uint32_t i;

	for(i=0; i < (elf->symtab_size/sizeof(elf_symbol)); i++)
	{
		if (ELF_ST_TYPE(elf->symtab[i].info) != 0x2)
			continue;
		if ((addr >= elf->symtab[i].value) && \
			(addr < (elf->symtab[i].value + elf->symtab[i].size)))
		{
			char *name = (char *)((uintptr_t)elf->strtab + \
				elf->symtab[i].name);
			return name;
		}
	}
	return 0;
}

