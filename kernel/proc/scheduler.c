#include <scheduler.h>
#include <k_debug.h>
#include <thread.h>
#include <process.h>
#include <stdint.h>
#include <lists.h>
#include <k_debug.h>
#include <synch.h>

list_head_t run_queue;

semaphore_t scheduler_sem;

void scheduler_insert(thread_t *th)
{
  spin_lock(&scheduler_sem);
  append_to_list(run_queue, th->tasks);
  spin_unlock(&scheduler_sem);
}

void scheduler_remove(thread_t *th)
{
  spin_lock(&scheduler_sem);
  remove_from_list(th->tasks);
  spin_unlock(&scheduler_sem);
}

thread_t *scheduler_next()
{
  // Returns the first thread on the run queue without removing it.

  if(run_queue.next != &run_queue)
    return list_entry(run_queue.next, thread_t, tasks);
  else
  {
    return 0;
    for(;;);
  }
}

void scheduler_init()
{
  // Setup the list of ready-to-run threads

  spin_lock(&scheduler_sem);
  init_list(run_queue);
  spin_unlock(&scheduler_sem);
}

void scheduler_list(list_head_t *list)
{
  // Print the threads on a list for debugging.
  // If no list is supplied, prints the run queue.

  if(list == 0)
    list = &run_queue;
  spin_lock(&scheduler_sem);
  list_t *i;
  for_each_in_list(list, i)
  {
    thread_t *th = list_entry(i, thread_t, tasks);
    debug("(%x:%x)->",th->proc->pid, th->tid);
  }
  spin_unlock(&scheduler_sem);
}

void scheduler_sleep(thread_t *th, list_head_t *list)
{
  // Removes a thread from the queue it's currently on and puts it to
  // sleep on the supplied queue.

  spin_lock(&scheduler_sem);
  remove_from_list(th->tasks);
  th->state = THREAD_STATE_WAITING;
  append_to_list(*list, th->tasks);
  spin_unlock(&scheduler_sem);
}

void scheduler_wake(list_head_t *list)
{
  // Wakes all threads on the supplied list and puts them in the run
  // queue.

  spin_lock(&scheduler_sem);
  list_t *i = list->next;;
  while(i != list)
  {
    thread_t *th = list_entry(i, thread_t, tasks);
    i = i->next;
    remove_from_list(th->tasks);
    append_to_list(run_queue, th->tasks);
    th->state = THREAD_STATE_READY;
  }
  spin_unlock(&scheduler_sem);
}

