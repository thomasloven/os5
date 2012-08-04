#include <thread.h>
#include <stdint.h>
#include <pmm.h>
#include <idt.h>
#include <vmm.h>
#include <heap.h>
#include <memory.h>

// If this line throws an error, the size of the kernel thread stack has grown too small. Please change MIN_THREAD_STACK_SIZE or thread_t in thread.h
uint32_t size_checker[1-2*!(THREAD_STACK_SPACE > MIN_THREAD_STACK_SIZE)];

uint32_t next_tid;

thread_info_t *current_thread_info()
{
	thread_info_t *ti;
	ti = (thread_info_t *)((uint32_t)&ti & PAGE_MASK);
	return ti;
}

thread_t *new_thread()
{
	thread_info_t *th_info = kvalloc(sizeof(thread_info_t));
	memset(&th_info->tcb, 0, sizeof(thread_t));

	th_info->tcb.tid = next_tid++;

	return &th_info->tcb;
}

thread_t *thread_init(uint32_t usermode_function_entry)
{

	next_tid = 1;

	thread_t *init = new_thread();

	init->r.eip = usermode_function_entry;
	init->r.useresp = USER_STACK_TOP;
	init->r.ebp = init->r.useresp;

	init->r.cs = SEG_USER_CODE | 0x3;
	init->r.ds = SEG_USER_DATA | 0x3;
	init->r.ss = SEG_USER_DATA | 0x3;

	return init;
}
