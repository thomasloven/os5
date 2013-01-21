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
	if(run_queue.next != &run_queue)
	{
		thread_t *new = list_entry(run_queue.next, thread_t, tasks);
		return new;
	} else {
		// Idle if there are no runnable threads
		for(;;);
	}
}

void scheduler_init()
{
	init_list(run_queue);
}

void scheduler_sleep(thread_t *th, list_head_t *on)
{
	scheduler_remove(th);
	append_to_list(*on, th->tasks);
}

void scheduler_wake(list_head_t *list, uint32_t value)
{
	list_t *item;
	while(!list_empty(*list))
	{
		item = list->next;
		thread_t *wake = list_entry(item, thread_t, tasks);
		wake->state = TH_STATE_RUNNING;
		remove_from_list(wake->tasks);
		wake->kernel_thread->eax = value;
		scheduler_insert(wake);
	}
}

