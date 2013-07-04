#include <k_syscall.h>
#include <arch.h>
#include <k_debug.h>
#include <synch.h>
#include <thread.h>
#include <process.h>

KDEF_SYSCALL(printf, r)
{
  process_stack stack = init_pstack();

  spin_lock(&debug_sem);
  debug((char *)stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6]);
  debug("<%x:%x>", current->proc->pid, current->tid);
  spin_unlock(&debug_sem);
  r->ebx = SYSCALL_OK;
  return r;
}
