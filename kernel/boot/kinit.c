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
#include <scheduler.h>

void myfunc(void)
{
	for(;;)
	{
		debug("A");
		asm volatile("int $0x82");
	}
}
void myfunc2(void)
{
	for(;;)
	{
		debug("B");
		asm volatile("int $0x82");
	}

}

registers_t *kinit(mboot_info_t *mboot, uint32_t mboot_magic)
{

	kdbg_init();
	pmm_init(mboot);
	idt_init();
	tss_init();
	scheduler_init();

	register_int_handler(INT_PF, page_fault_handler);
	register_int_handler(0x82, switch_kernel_thread);

	debug("Starting up thread");

	thread_t *init = thread_init((uint32_t)&myfunc);
	thread_t *th2 = thread_init((uint32_t)&myfunc2);
	scheduler_remove(init);

	init->r.eflags = EFL_INT;

	set_kernel_stack(stack_from_tcb(init));

	debug("Starting up thread");


	return &init->r;
}

