#include <stdint.h>
#include <pmm.h>
#include <vmm.h>
#include <multiboot.h>
#include <ctype.h>
#include <k_debug.h>

uint8_t pmm_running;
uintptr_t pmm_pos;

uintptr_t pmm_page_stack_max = PMM_PAGE_STACK;
uintptr_t pmm_page_stack_ptr = PMM_PAGE_STACK;

uintptr_t pmm_alloc_page()
{
	if(pmm_running == FALSE)
	{
		// If pmm isn't running, just hand out space from the end of
		// the kernel. This should happen, like, once.
		pmm_pos += PAGE_SIZE;
		return pmm_pos - PAGE_SIZE;
	}

	if ( pmm_page_stack_ptr == PMM_PAGE_STACK)
	{
		// This is where swapping memory pages to disk should go
		panic("Out of memory!");
	}

	pmm_page_stack_ptr = pmm_page_stack_ptr - sizeof(uintptr_t *);

	if(pmm_page_stack_ptr <= (pmm_page_stack_max - PAGE_SIZE))
	{
		// If an entire page of stack is cleared, hand out that page
		pmm_page_stack_max = pmm_page_stack_max - PAGE_SIZE;
		uintptr_t ret = vmm_page_get(pmm_page_stack_max - PAGE_SIZE) & \
			PAGE_MASK;
		
		vmm_page_set(ret, 0);
		return ret;
	}
	// Hand out the page at the top of the stack
	uintptr_t *stack = (uintptr_t *)pmm_page_stack_ptr;
	return *stack;
}

void pmm_free_page(uintptr_t page)
{
	page &= PAGE_MASK;

	if(page < pmm_pos)
		return;

	if(pmm_page_stack_ptr >= pmm_page_stack_max)
	{
		// If stack is full, use this page to increase it
		vmm_page_set(pmm_page_stack_max, \
			vmm_page_val(page, PAGE_PRESENT | PAGE_WRITE));
		pmm_page_stack_max = pmm_page_stack_max + PAGE_SIZE;
	} else {
		// Push page onto stack
		uintptr_t *stack = (uintptr_t *)pmm_page_stack_ptr;
		*stack = page;
		pmm_page_stack_ptr = pmm_page_stack_ptr + sizeof(uintptr_t *);
	}
	pmm_running = TRUE;
}

void pmm_init(mboot_info_t *mboot)
{
	// Start of memory
	pmm_pos = (mboot->mem_upper + PAGE_SIZE) & PAGE_MASK;
	pmm_running = FALSE;

	// Read memory map from multiboot data
	uintptr_t i = assert_higher(mboot->mmap_addr);
	while (i < mboot->mmap_addr + mboot->mmap_length)
	{
		mboot_mmap_entry_t *me = (mboot_mmap_entry_t *)i;
		if(me->type == MBOOT_MEM_FLAG_FREE)
		{
			// Push free pages onto page stack
			uint32_t j;
			for(j = me->base_addr_lower; \
				j <= (me->base_addr_lower + me->length_lower); j += PAGE_SIZE)
			{
				pmm_free_page(j);
			}
		}
		i += me->size + sizeof(uint32_t);
	}
}
