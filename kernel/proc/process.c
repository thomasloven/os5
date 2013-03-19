#include <stdint.h>
#include <process.h>
#include <memory.h>
#include <k_heap.h>
#include <thread.h>
#include <lists.h>
#include <vmm.h>
#include <procmm.h>
#include <k_debug.h>
#include <scheduler.h>

uint32_t next_pid = 1;

list_head_t process_list;

process_t *alloc_process()
{
  process_t *p = kmalloc(sizeof(process_t));
  memset(p, 0, sizeof(process_t));

  p->pid = next_pid++;

  init_list(p->threads);
  init_list(p->proc_list);
  init_list(p->waiting);
  init_list(p->signals);

  int i;
  for(i = 0; i < NUM_SIGNALS; i++)
  {
    p->signal_actions[i] = SIG_DFL;
    p->signal_mask[i] = 1;
  }

  append_to_list(process_list, p->proc_list);

  init_procmm(p);

  return p;

}

process_t *get_process(uint32_t pid)
{
  list_t *i;
  for_each_in_list(&process_list, i)
  {
    process_t *p = list_entry(i, process_t, proc_list);
    if(p->pid == pid)
      return p;
  }
  return 0;
}

void free_process(process_t *proc)
{

  list_t *i = (&proc->threads)->next;
  while (i != &proc->threads)
  {
    thread_t *th = list_entry(i, thread_t, process_threads);
    i = i->next;
    if(th != current)
    free_thread(th);
  }

  // Make init adopt all the processes children
  process_t *init = list_entry(process_list.next, process_t, proc_list);
  if(proc->child)
  {
    process_t *ch = proc->child;
    process_t *i = ch;
    while(i->older_sibling)
    {
      i->parent = init;
      i = i->older_sibling;
    }
    i->older_sibling = init->child;
    i->older_sibling->younger_sibling = i;
    init->child = ch;
  }

  remove_from_list(proc->proc_list);

  procmm_exit(proc);

  kfree(proc);
}

void exit_process(process_t *proc, uint32_t exit_code)
{
  proc->exit_code = exit_code;
  list_t *i = (&proc->threads)->next;
  while (i != &proc->threads)
  {
    thread_t *th = list_entry(i, thread_t, process_threads);
    i = i->next;
    if(th != current)
    free_thread(th);
  }

  proc->state = PROC_STATE_FINISHED;

  current->state = THREAD_STATE_FINISHED;

  scheduler_wake(&proc->waiting);
}

void switch_process(process_t *proc)
{
  vmm_pd_set(proc->pd);
}


process_t *process_init(void (*func)(void))
{

  init_list(process_list);

  process_t *proc = alloc_process();
  proc->pd = vmm_clone_pd();

  thread_t *th = new_thread(func,0);
  append_to_list(proc->threads, th->process_threads);
  th->proc = proc;

  th->r.eflags = EFL_INT;
  set_kernel_stack(stack_from_tcb(th));

  boot_thread = thinfo_from_tcb(th);

  switch_process(proc);

  return proc;
}

process_t *fork_process()
{
  process_t *parent = current->proc;
  process_t *child = alloc_process();

  // Clone page directory
  child->pd = vmm_clone_pd();
  // Clone memory map (copy on write for everything)
  procmm_fork(parent, child);

  memcopy(child->signal_actions, parent->signal_actions, NUM_SIGNALS*sizeof(uintptr_t));
  memcopy(child->signal_mask, parent->signal_mask, NUM_SIGNALS*sizeof(uint8_t));

  // Fix the family
  child->parent = parent;
  child->older_sibling = parent->child;
  if(child->older_sibling)
    child->older_sibling->younger_sibling = child;
  parent->child = child;

  thread_t *th = clone_thread(current);
  append_to_list(child->threads, th->process_threads);
  th->proc = child;

  return child;
}

void signal_process(process_t *p, uint32_t signum)
{
  debug("Sending signal %x to %x", signum, p->pid);
  signal_t *sig = kmalloc(sizeof(signal_t));
  init_list(sig->signals);
  sig->signum = signum;

  append_to_list(p->signals, sig->signals);
}

thread_t *handle_signal(thread_t *old)
{
  process_t *p = old->proc;

  if(list_empty(p->signals))
    return old;
  
  signal_t *sig = list_entry(p->signals.next, signal_t, signals);
  remove_from_list(sig->signals);

  if(p->signal_actions[sig->signum] == SIG_IGNORE)
  {
    kfree(sig);
    return old;
  } else if (p->signal_actions[sig->signum] == SIG_DFL) {
    exit_process(p, sig->signum);
    return 0;
  } else {

  }


  return old;
}

