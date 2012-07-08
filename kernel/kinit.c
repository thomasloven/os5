#include <stdint.h>
#include <k_debug.h>
#include <pmm.h>
#include <multiboot.h>
#include <heap.h>
#include <idt.h>
#include <arch.h>
#include <memory.h>
#include <debug.h>
#include <thread.h>

void myfunc(void)
{
	debug("It worked!");
	debug("\ncurrent:%x", current.tid);
	for(;;)asm("hlt");
}

registers_t *kinit(mboot_info_t *mboot, uint32_t mboot_magic)
{

	kdbg_init();
	pmm_init(mboot);
	idt_init();
	debug("Hello, world!\n%x%d",0x123ABC, 123);
	debug("\nheader%x\nmagic%x",mboot, mboot_magic);



	enable_interrupts();
	thread_info_t *stk = kvalloc(sizeof(thread_info_t));
	debug("\n Stack position:%x", (uint32_t)stk);
	memset(stk, 0, sizeof(thread_info_t));

	stk->tcb.tid = 0xABABAB;
	stk->tcb.r.eip = &myfunc;
	stk->tcb.r.esp = kmalloc(0x2000) + 0x1FF0;
	stk->tcb.r.ebp = stk->tcb.r.esp;
	stk->tcb.r.ds = 0x10;
	stk->tcb.r.cs = 0x08;
	/*stk->tcb.r.eflags = 1<<9;*/
	return &stk->tcb.r;
	assert(3<2);


}

