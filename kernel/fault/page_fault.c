#include <stdint.h>
#include <idt.h>
#include <vmm.h>
#include <k_debug.h>
#include <thread.h>
#include <arch.h>

registers_t *page_fault_handler(registers_t *r)
{
	uint32_t fault_address;
	__asm__ volatile("mov %%cr2, %0" : "=r" (fault_address));
	if(!(r->cs & 0x3))
	{

		if(fault_address >= KERNEL_OFFSET && !(r->err_code & PF_PRESENT))
		{
			page_dir_t old_exdir = vmm_exdir_set(kernel_pd);
			uintptr_t pt_val = vmm_extable_get(fault_address);
			if(pt_val & PAGE_PRESENT)
			{
				// Kernel tried to access a page table that's not in the current page
				// directory but exists in the master directory. So just copy it over.
				
				vmm_table_set(fault_address, pt_val);
			}
			vmm_exdir_set(old_exdir);
		}

		// Processor was in kernel mode
		disable_interrupts();
		debug("Page fault in kernel!");
		debug("\n At: %x", fault_address);
		print_registers(r);
		print_stack_trace();
		for(;;);

	} else {

		// Processor was in user mode
		if((fault_address <=USER_STACK_TOP) && (fault_address >=USER_STACK_BOTTOM))
		{
			// If the page fault was in the stack area of a thread, allocate a page nd return.
			vmm_page_set(fault_address & PAGE_MASK, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_WRITE | PAGE_USER));
			return r;

		} else {

			disable_interrupts();
			debug("Page fault hapened!");
			debug("\n At: %x", fault_address);
			debug("\n From thread: %x", current->tid);
			for(;;);
		}

	}
}
