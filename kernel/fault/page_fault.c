#include <stdint.h>
#include <idt.h>
#include <vmm.h>
#include <k_debug.h>
#include <thread.h>
#include <arch.h>
#include <elf.h>
#include <process.h>

void print_user_stack(registers_t *r, elf_t *elf)
{
	uintptr_t *esp = (uintptr_t *)r->useresp;
	while(esp < (uintptr_t *)0xC0000000)
	{
		if(elf_lookup_symbol(elf, *esp))
			debug("\n [%x] %s",esp, elf_lookup_symbol(elf, *esp));
		esp ++;
	}
}

registers_t *page_fault_handler(registers_t *r)
{
	uint32_t fault_address;
	__asm__ volatile("mov %%cr2, %0" : "=r" (fault_address));
	if(!(r->cs & 0x3))
	{
		disable_interrupts();
		debug("Page fault in kernel!");
		debug("\n At: %x", fault_address);
		print_registers(r);
		print_stack_trace(r);
		for(;;);
	} else {

		if((fault_address <=USER_STACK_TOP) && \
			(fault_address >=USER_STACK_BOTTOM))
		{
			// If the page fault was in the stack area of a thread, 
			//	allocate a page and return.
			vmm_page_set(fault_address & PAGE_MASK, \
				vmm_page_val(pmm_alloc_page(), \
				PAGE_PRESENT | PAGE_WRITE | PAGE_USER));
			return r;

		} else {

			disable_interrupts();
			debug("Page fault hapened!");
			debug("\n At: %x", fault_address);
			debug("\n From thread: %x", current);
			print_registers(r);
			debug("\n              %s",elf_lookup_symbol(&current->proc->elf, r->eip));
			print_user_stack(r, &current->proc->elf);
			for(;;);
		}
	}
}

