#include <thread.h>
#include <k_debug.h>
#include <stdint.h>
#include <pmm.h>
#include <idt.h>
#include <vmm.h>
#include <heap.h>
#include <memory.h>
#include <scheduler.h>
#include <lists.h>

// If this line throws an error, the size of the kernel thread stack 
// has grown too small. Please change MIN_THREAD_STACK_SIZE or thread_t 
// in thread.h. Or rather yet, change how the stack is allocated so that 
// it works for all sizes...
uint32_t size_checker[1-2*!(THREAD_STACK_SPACE > MIN_THREAD_STACK_SIZE)];

uint32_t next_tid=1;

thread_info_t *current_thread_info()
{
	thread_info_t *ti;
	ti = (thread_info_t *)((uint32_t)&ti & PAGE_MASK);
	return ti;
}

thread_t *alloc_thread()
{
	thread_info_t *th_info = kvalloc(sizeof(thread_info_t));
	memset(&th_info->tcb, 0, sizeof(thread_t));

	th_info->tcb.tid = next_tid++;

	init_list(th_info->tcb.proc_threads);
	init_list(th_info->tcb.tasks);

	return &th_info->tcb;
}

thread_t *threads_init(void *func)
{
	thread_t *idle = alloc_thread();
	idle->r.eip = (uint32_t)func;

	idle->r.cs = SEG_KERNEL_CODE;
	idle->r.ds = SEG_KERNEL_DATA;
	idle->r.ss = SEG_KERNEL_DATA;

	idle->r.eflags = EFL_INT;

	set_kernel_stack(stack_from_tcb(idle));

	scheduler_insert(idle);
	idle->kernel_thread = &idle->r;
	return idle;
}

thread_t *new_thread(void *func, uint8_t user)
{
	thread_t *th = alloc_thread();

	th->r.eip = (uint32_t)func;
	if(user)
	{
		th->r.useresp = USER_STACK_TOP;
		th->r.ebp = th->r.useresp;
		th->r.ebp = 0;

		th->r.cs = SEG_USER_CODE | 0x3;
		th->r.ds = SEG_USER_DATA | 0x3;
		th->r.ss = SEG_USER_DATA | 0x3;
	} else {
		th->r.cs = SEG_KERNEL_CODE;
		th->r.ds = SEG_KERNEL_DATA;
		th->r.ss = SEG_KERNEL_DATA;
	}

	scheduler_insert(th);

	th->kernel_thread = &th->r;

	debug("\n Current %x", current->proc);
	th->proc = current->proc;
	append_to_list(th->proc->threads,th->proc_threads);

	return th;
}

registers_t *switch_kernel_thread(registers_t *r)
{
	if(r)
	{
		current->kernel_thread = r;
		scheduler_insert(current);
	}

	thread_t *next = scheduler_next();
	scheduler_remove(next);
	switch_process(next->proc);

	return next->kernel_thread;
}

