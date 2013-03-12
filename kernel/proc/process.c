#include <stdint.h>
#include <process.h>
#include <memory.h>
#include <heap.h>
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

  append_to_list(process_list, p->proc_list);

  init_procmm(p);

  return p;

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

  thread_t *th = clone_thread(current);
  append_to_list(child->threads, th->process_threads);
  th->proc = child;

  // Fix the family
  child->parent = parent;
  child->older_sibling = parent->child;
  if(child->older_sibling)
    child->older_sibling->younger_sibling = child;
  parent->child = child;

  return child;
}

