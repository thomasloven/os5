#include <thread.h>
#include <stdint.h>
#include <pmm.h>

// If this line throws an error, the size of the kernel thread stack has grown too small. Please change MIN_THREAD_STACK_SIZE or thread_t in thread.h
uint32_t size_checker[1-2*!(THREAD_STACK_SPACE > MIN_THREAD_STACK_SIZE)];

thread_info_t *current_thread_info()
{
	thread_info_t *ti;
	ti = (thread_info_t *)((uint32_t)&ti & PAGE_MASK);
	return ti;
}
