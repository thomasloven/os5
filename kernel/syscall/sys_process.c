#include <k_syscall.h>
#include <syscall.h>
#include <process.h>
#include <k_debug.h>
#include <scheduler.h>
#include <thread.h>

KDEF_SYSCALL(fork, r)
{
  process_t *child = fork_process();
  thread_t *cth = list_entry(child->threads.next, thread_t, process_threads);
  cth->r.eax = 0;
  r->eax = child->pid;
  r->ebx = cth->r.ebx = SYSCALL_OK;

  scheduler_insert(cth);
  return r;
}

KDEF_SYSCALL(getpid, r)
{
  r->eax = current->proc->pid;
  r->ebx = SYSCALL_OK;
  return r;
}

KDEF_SYSCALL(exit, r)
{
  process_stack stack = init_pstack();

  exit_process(current->proc, stack[0]);

  current->state = THREAD_STATE_FINISHED;

  schedule();

  debug("ERROR! REACHED END OF EXIT SYSCALL!");
  for(;;);
  return r;
}

KDEF_SYSCALL(wait, r)
{
  process_stack stack = init_pstack();

  process_t *proc = get_process(stack[0]);

  while(proc->state != PROC_STATE_FINISHED)
  {
    scheduler_sleep(current, &proc->waiting);
    
    schedule();
  }

  r->eax = proc->exit_code;
  free_process(proc);

  r->ebx = SYSCALL_OK;
  return r;
}

