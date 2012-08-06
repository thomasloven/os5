#include <scheduler.h>
#include <k_debug.h>
#include <thread.h>
#include <stdint.h>
#include <lists.h>

list_head_t run_queue;

void scheduler_insert(thread_t *th)
{
	append_to_list(run_queue, th->tasks);
}

void scheduler_remove(thread_t *th)
{
	remove_from_list(th->tasks);
}

thread_t *scheduler_next()
{
	if(run_queue.next)
		return list_entry(run_queue.next, thread_t, tasks);
	else
	{
		for(;;);
	}
}

void scheduler_init()
{
	init_list(run_queue);
}

