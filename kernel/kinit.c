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


	thread_info_t *init = kvalloc(sizeof(thread_info_t));
	memset(init, 0, sizeof(thread_info_t));

	uint32_t usermode_function_entry = 0x10000;
	vmm_page_set(usermode_function_entry, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_WRITE | PAGE_USER));
	memcopy(usermode_function_entry, &myfunc, 0x100);

	init->tcb.r.eip = usermode_function_entry;

	init->tcb.r.useresp = USER_STACK_TOP;
	init->tcb.r.ebp = init->tcb.r.useresp;

	init->tcb.r.cs = SEG_USER_CODE | 0x3;
	init->tcb.r.ds = SEG_USER_DATA | 0x3;
	init->tcb.r.ss = SEG_USER_DATA | 0x3;

	init->tcb.r.eflags = EFL_INT;

	set_kernel_stack(thread_info_stack(init));

	return &init->tcb.r;
}

