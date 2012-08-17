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
	// Find the current thread structure from stack pointer
	thread_info_t *ti;
	ti = (thread_info_t *)((uint32_t)&ti & PAGE_MASK);
	return ti;
}

thread_t *alloc_thread()
{
	// Create a new thread
	thread_info_t *th_info = kvalloc(sizeof(thread_info_t));
	thread_t *th = &th_info->tcb;
	memset(th, 0, sizeof(thread_t));

	th->tid = next_tid++;

	init_list(th->proc_threads);
	init_list(th->tasks);

	return th;
}

thread_t *new_thread(uint8_t user, uint32_t userstack)
{
	// Create a new kernel or usermode thread
	// with entry point at func
	thread_t *th = alloc_thread();

	if(user)
	{
		th->r.useresp = (userstack)?userstack:USER_STACK_TOP;
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

	th->kernel_thread = &th->r;

	return th;
}

void free_thread(thread_t *th)
{
	scheduler_remove(th);
	unbind_thread(th);
	kfree(thinfo_from_tcb(th));
}

void bind_thread(thread_t *th, process_t *p)
{
	th->proc = p;
	append_to_list(p->threads, th->proc_threads);
}

void unbind_thread(thread_t *th)
{
	remove_from_list(th->proc_threads);
	th->proc = 0;
}

void thread_execute(thread_t *th, void *func)
{
	th->r.eip = (uint32_t) func;
}

thread_t *add_thread(void *func, uint8_t user)
{
	// Adds a thread to the current process
	thread_t *th = new_thread(user, 0);

	th->r.eip = (uint32_t)func;

	scheduler_insert(th);

	bind_thread(th, current->proc);

	return th;
}

registers_t *switch_kernel_thread(registers_t *r)
{
	// Syscall for switching threads in the kernel
	// Callable only from kernel mode
	if(r)
	{
		current->kernel_thread = r;
		scheduler_insert(current);
	}

	thread_t *next = scheduler_next();
	while(next->state == TH_STATE_FINISHED)
	{
		process_t *p = next->proc;
		free_thread(next);
		if(list_empty(p->threads))
			free_process(p);
		next = scheduler_next();
	}
	scheduler_remove(next);
	switch_process(next->proc);

	return next->kernel_thread;
}

