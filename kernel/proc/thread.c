#include <thread.h>
#include <ctype.h>
#include <k_debug.h>
#include <stdint.h>
#include <pmm.h>
#include <idt.h>
#include <vmm.h>
#include <memory.h>
#include <scheduler.h>
#include <process.h>
#include <arch.h>

#include <stdlib.h>
#include <malloc.h>

// If this line throws an error, the size of the kernel thread stack has grown too small. Please change MIN_THREAD_STACK_SIZE or thread_t in thread.h
// Or rather yet, change how the stack is allocated so that it works for all sizes...
uint32_t size_checker[1-2*!(THREAD_STACK_SPACE > MIN_THREAD_STACK_SIZE)];

uint32_t next_tid=1;

thread_info_t *current_thread_info()
{
  if(!kernel_booted)
    return boot_thread;

  thread_info_t *ti;
  ti = (thread_info_t *)((uint32_t)&ti & PAGE_MASK);
  return ti;
}

thread_t *alloc_thread()
{
  thread_info_t *th_info = valloc(sizeof(thread_info_t));
  memset(&th_info->tcb, 0, sizeof(thread_t));

  th_info->tcb.tid = next_tid++;

  init_list(th_info->tcb.tasks);
  init_list(th_info->tcb.process_threads);

  return &th_info->tcb;
}

void free_thread(thread_t *th)
{

  scheduler_remove(th);

  remove_from_list(th->tasks);
  remove_from_list(th->process_threads);

  thread_info_t *th_info = thinfo_from_tcb(th);
  free(th_info);
}

thread_t *new_thread(void (*func)(void), uint8_t user)
{
  thread_t *th = alloc_thread();

  th->r.eip = (uint32_t)func;
  if(user)
  {
    th->r.useresp = USER_STACK_TOP;
    th->r.ebp = th->r.useresp;

    th->r.cs = SEG_USER_CODE | 0x3;
    th->r.ds = SEG_USER_DATA | 0x3;
    th->r.ss = SEG_USER_DATA | 0x3;
  } else {
    th->r.cs = SEG_KERNEL_CODE;
    th->r.ds = SEG_KERNEL_DATA;
    th->r.ss = SEG_KERNEL_DATA;
  }

  th->state = THREAD_STATE_READY;
  scheduler_insert(th);

  th->kernel_thread = &th->r;

  return th;
}

thread_t *clone_thread(thread_t *th)
{
  thread_t *new = alloc_thread();

  uint32_t tid = new->tid;
  memcpy(new, th, sizeof(thread_t));
  new->tid = tid;

  init_list(new->tasks);
  init_list(new->process_threads);

  new->kernel_thread = &new->r;

  return new;
}

thread_t *handle_signals(thread_t *th)
{
  if(!list_empty(th->proc->signal_queue))
  {
    // There are signals that need handling.
    signal_t *signal = list_entry(th->proc->signal_queue.next, signal_t, queue);
    debug("Need to handle signal %d", signal->sig);
  }
  return th;
}

registers_t *switch_kernel_thread(registers_t *r)
{
  if(r && r != &boot_thread->tcb.r)
  {
    current->kernel_thread = r;
    if(current->state == THREAD_STATE_READY)
      scheduler_insert(current);
  }

  thread_t *next = scheduler_next();
  if(!next)
    next = &boot_thread->tcb;
  else
    scheduler_remove(next);

  switch_process(next->proc);

  next = handle_signals(next);

  kernel_booted = TRUE;

  return next->kernel_thread;
}

