#include <stdint.h>
#include <idt.h>
#include <vmm.h>
#include <k_debug.h>
#include <thread.h>

registers_t *page_fault_handler(registers_t *r)
{
	uint32_t fault_address;
	asm volatile("mov %%cr2, %0" : "=r" (fault_address));
	if(!(r->cs & 0x3))
	{
		debug("Page fault in kernel!");
		debug("\n At: %x", fault_address);
		print_registers(r);
		for(;;);
	} else {

		if((fault_address <=USER_STACK_TOP) && (fault_address >=USER_STACK_BOTTOM))
		{
			// If the page fault was in the stack area of a thread, allocate a page nd return.
			vmm_page_set(fault_address & PAGE_MASK, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_WRITE | PAGE_USER));
			return r;

		} else {

			debug("Page fault hapened!");
			debug("\n At: %x", fault_address);
			debug("\n From thread: %x", current->tid);
			for(;;);
		}
	}
}
