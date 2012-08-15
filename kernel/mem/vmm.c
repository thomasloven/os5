#include <vmm.h>
#include <pmm.h>
#include <stdint.h>
#include <memory.h>
#include <k_debug.h>

uintptr_t *page_directory = (uintptr_t *)VMM_PAGE_DIR;
uintptr_t *page_tables = (uintptr_t *)VMM_PAGE_TABLES;
uintptr_t *expage_directory = (uintptr_t *)VMM_EXPAGE_DIR;
uintptr_t *expage_tables = (uintptr_t *)VMM_EXPAGE_TABLES;

uintptr_t vmm_page_get(uintptr_t page)
{
	// Get the page table entry, if it exists
	page &= PAGE_MASK;

	if(page_directory[vmm_dir_idx(page)] & PAGE_PRESENT)
		if(page_tables[vmm_table_idx(page)] & PAGE_PRESENT)
			return page_tables[vmm_table_idx(page)];
	return 0;
}

void vmm_page_touch(uintptr_t page, uint32_t flags)
{
	// Make sure a page table exists for the passed page
	page &= PAGE_MASK;
	flags &= PAGE_FLAG_MASK;

	if(!(page_directory[vmm_dir_idx(page)] & PAGE_PRESENT))
	{
		page_directory[vmm_dir_idx(page)] = pmm_alloc_page() | flags;
		vmm_flush_tlb((uintptr_t)&page_tables[vmm_table_idx(page)] & \
			PAGE_MASK);
		memset((uintptr_t)&page_tables[vmm_table_idx(page)] & PAGE_MASK, \
			0, PAGE_SIZE);
	}
}

void vmm_page_set(uintptr_t page, uintptr_t value)
{
	// Set page table entry
	page &= PAGE_MASK;

	vmm_page_touch(page, value & PAGE_FLAG_MASK);
	page_tables[vmm_table_idx(page)] = value;
	vmm_flush_tlb(page);
}


void vmm_exdir_set(uintptr_t dir)
{
	// Load an external page directory
	if(dir)
		page_directory[vmm_dir_idx(VMM_EXPAGE_DIR)] = \
			dir | PAGE_PRESENT | PAGE_WRITE;
	else
		page_directory[vmm_dir_idx(VMM_EXPAGE_DIR)] = 0;
}

uintptr_t vmm_expage_get(uintptr_t page)
{
	// Get page table entry for external pagedir
	page &= PAGE_MASK;

	if(expage_directory[vmm_dir_idx(page)] & PAGE_PRESENT)
		if(expage_tables[vmm_table_idx(page)] & PAGE_PRESENT)
			return expage_tables[vmm_table_idx(page)];
	return 0;
}

void vmm_expage_touch(uintptr_t page, uint32_t flags)
{
	// Make sure a page table exists for external pagedir
	page &= PAGE_MASK;
	flags &= PAGE_FLAG_MASK;

	if(!(expage_directory[vmm_dir_idx(page)] & PAGE_PRESENT))
	{
		expage_directory[vmm_dir_idx(page)] = pmm_alloc_page() | flags;
		vmm_flush_tlb((uintptr_t)&expage_tables[vmm_table_idx(page)] & \
			PAGE_MASK);
		memset((uintptr_t)&expage_tables[vmm_table_idx(page)] & PAGE_MASK, \
			0, PAGE_SIZE);
	}
}

void vmm_expage_set(uintptr_t page, uintptr_t value)
{
	// Set page table entry for external pagedir
	page &= PAGE_MASK;

	vmm_expage_touch(page, value & PAGE_FLAG_MASK);
	expage_tables[vmm_table_idx(page)] = value;
}

uintptr_t vmm_new_pd()
{
	// Get a new page directory
	uintptr_t pd = pmm_alloc_page();

	vmm_page_set(VMM_TEMP1, vmm_page_val(pd, PAGE_PRESENT | PAGE_WRITE));
	vmm_flush_tlb(VMM_TEMP1);

	// Zero it
	memset(VMM_TEMP1, 0, PAGE_SIZE);

	// Make it recursive
	uint32_t *pdir = (uint32_t *) VMM_TEMP1;
	pdir[VMM_PAGES_PER_TABLE -1] = vmm_page_val(pd, PAGE_PRESENT | PAGE_WRITE);

	vmm_page_set(VMM_TEMP1, 0);
	vmm_flush_tlb(VMM_TEMP1);

	return pd;
}

void vmm_clear_page(uintptr_t page)
{
	// zero out a physical page
	page &= PAGE_MASK;

	vmm_page_set(VMM_TEMP1, vmm_page_val(page, PAGE_PRESENT | PAGE_WRITE));
	memset(VMM_TEMP1, 0, PAGE_SIZE);
	vmm_page_set(VMM_TEMP1, 0);
}

void vmm_copy_page(uintptr_t dst, uintptr_t src)
{
	// Copy a physical page
	dst &= PAGE_MASK;
	src &= PAGE_MASK;

	vmm_page_set(VMM_TEMP1, vmm_page_val(dst, PAGE_PRESENT | PAGE_WRITE));
	vmm_page_set(VMM_TEMP2, vmm_page_val(src, PAGE_PRESENT));
	memcopy(VMM_TEMP1, VMM_TEMP2, PAGE_SIZE);
	vmm_page_set(VMM_TEMP1, 0);
	vmm_page_set(VMM_TEMP2, 0);
}

uintptr_t vmm_clone_pd()
{
	// Make a copy of the current pagedir

	uintptr_t pd = vmm_new_pd();
	vmm_exdir_set(pd);

	uint32_t table;
	for(table = 0; table < vmm_dir_idx(KERNEL_OFFSET); table++)
	{
		// User space
		if(page_directory[table])
		{
			// Copy page table
			expage_directory[table] = \
				vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_WRITE);
			vmm_clear_page(expage_directory[table] & PAGE_MASK);
			uint32_t page;
			for(page = 0; page < VMM_PAGES_PER_TABLE; page++)
			{
				// Copy each page in the table
				// Sooner or later I should implement copy on write
				uint32_t entry = table * VMM_PAGES_PER_TABLE + page;
				if(page_tables[entry])
				{
					expage_tables[entry] = vmm_page_val(pmm_alloc_page(), \
						page_tables[entry] & PAGE_FLAG_MASK);
					vmm_copy_page(expage_tables[entry] & PAGE_MASK, \
						page_tables[entry] & PAGE_MASK);
				}
			}
		}
	}

	// Don't copy kernel space, just link it.
	//
	// Right now changes in kernel space are not synced.
	// High priority on that one.
	for(table = vmm_dir_idx(KERNEL_OFFSET); table < VMM_PAGES_PER_TABLE -2; table++)
	{
		expage_directory[table] = page_directory[table];
	}
	expage_directory[vmm_dir_idx(VMM_EXPAGE_DIR)] = 0;
	vmm_exdir_set(0);
	return pd;
}

