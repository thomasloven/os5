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
  spin_lock(&scheduler_sem);
  init_list(run_queue);
  spin_unlock(&scheduler_sem);
}

void scheduler_list(list_head_t *list)
{
  if(list == 0)
    list = &run_queue;
  spin_lock(&scheduler_sem);
  list_t *i;
  for_each_in_list(list, i)
  {
    thread_t *th = list_entry(i, thread_t, tasks);
    debug("%x->",th->tid);
  }
  spin_unlock(&scheduler_sem);
}

void scheduler_sleep(thread_t *th, list_head_t *list)
{
  spin_lock(&scheduler_sem);
  remove_from_list(th->tasks);
  th->state = THREAD_STATE_WAITING;
  append_to_list(*list, th->tasks);
  spin_unlock(&scheduler_sem);
}

void scheduler_wake(list_head_t *list)
{
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

