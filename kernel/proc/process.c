#include <stdint.h>
#include <process.h>
#include <memory.h>
#include <heap.h>
#include <thread.h>
#include <lists.h>
#include <vmm.h>

uint32_t next_pid = 1;

list_head_t process_list;

process_t *alloc_process()
{
	process_t *p = kmalloc(sizeof(process_t));
	memset(p, 0, sizeof(process_t));

	p->pid = next_pid++;

	init_list(p->threads);
	init_list(p->proc_list);

	append_to_list(process_list, p->proc_list);

	return p;

}


void process_init(void (*func)(void))
{

	init_list(process_list);

	process_t *proc = alloc_process();

	thread_t *th = new_thread(func,0);
	append_to_list(proc->threads, th->process_threads);

	th->r.eflags = EFL_INT;
	set_kernel_stack(stack_from_tcb(th));

}
