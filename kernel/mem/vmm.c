#include <vmm.h>
#include <pmm.h>
#include <stdint.h>
#include <memory.h>

uintptr_t *page_directory = (uintptr_t *)PMM_PAGE_DIR;
uintptr_t *page_tables = (uintptr_t *)PMM_PAGE_TABLES;

uintptr_t vmm_page_get(uintptr_t page)
{
	page &= PAGE_MASK;

	if(page_directory[vmm_dir_idx(page)] & PAGE_PRESENT)
		if(page_tables[vmm_table_idx(page)] & PAGE_PRESENT)
			return page_tables[vmm_table_idx(page)];
	return 0;
}

void vmm_page_touch(uintptr_t page, uint32_t flags)
{
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
	page &= PAGE_MASK;

	vmm_page_touch(page, value & PAGE_FLAG_MASK);
	page_tables[vmm_table_idx(page)] = value;
	vmm_flush_tlb(page);
}
