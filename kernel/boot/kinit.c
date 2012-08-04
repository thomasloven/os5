#include <stdint.h>
#include <k_debug.h>
#include <pmm.h>
#include <vmm.h>
#include <multiboot.h>
#include <heap.h>
#include <idt.h>
#include <arch.h>
#include <memory.h>
#include <debug.h>
#include <thread.h>

void myfunc(void)
{
	uint32_t localvar = 0xABC;
	asm volatile("jmp *%0" : : "r" (localvar));
	for(;;);
}

registers_t *kinit(mboot_info_t *mboot, uint32_t mboot_magic)
{

	kdbg_init();
	pmm_init(mboot);
	idt_init();
	tss_init();

	register_int_handler(INT_PF, page_fault_handler);

	uint32_t usermode_function_entry = 0x10000;
	vmm_page_set(usermode_function_entry, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_WRITE | PAGE_USER));
	memcopy(usermode_function_entry, &myfunc, 0x100);

	thread_t *init = thread_init(usermode_function_entry);

	init->r.eflags = EFL_INT;

	set_kernel_stack(stack_from_tcb(init));

	return &init->r;
}

