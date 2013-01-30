#include <thread.h>
#include <k_debug.h>
#include <stdint.h>
#include <pmm.h>
#include <idt.h>
#include <vmm.h>
#include <heap.h>
#include <memory.h>
#include <scheduler.h>

// If this line throws an error, the size of the kernel thread stack has grown too small. Please change MIN_THREAD_STACK_SIZE or thread_t in thread.h
// Or rather yet, change how the stack is allocated so that it works for all sizes...
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

	init_list(th_info->tcb.tasks);
	init_list(th_info->tcb.process_threads);

	return &th_info->tcb;
}

thread_t *new_thread(void (*func)(void), uint8_t user)
{
	thread_t *init = alloc_thread();

	init->r.eip = (uint32_t)func;
	if(user)
	{
		init->r.useresp = USER_STACK_TOP;
		init->r.ebp = init->r.useresp;

		init->r.cs = SEG_USER_CODE | 0x3;
		init->r.ds = SEG_USER_DATA | 0x3;
		init->r.ss = SEG_USER_DATA | 0x3;
	} else {
		init->r.cs = SEG_KERNEL_CODE;
		init->r.ds = SEG_KERNEL_DATA;
		init->r.ss = SEG_KERNEL_DATA;
	}

	scheduler_insert(init);

	init->kernel_thread = &init->r;

	return init;
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

	return next->kernel_thread;
}

