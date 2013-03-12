#include <k_syscall.h>
#include <syscall.h>
#include <process.h>

KDEF_SYSCALL(fork, r)
{
  process_t *child = fork_process();
  thread_t *cth = list_entry(child->threads.next, thread_t, process_threads);
  cth->r.eax = 0;
  r->eax = child->pid;
  r->ebx = cth->r.ebx = SYSCALL_OK;
  return r;
}

KDEF_SYSCALL(getpid, r)
{
  r->eax = current->proc->pid;
  r->ebx = SYSCALL_OK;
  return r;
}
