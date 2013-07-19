#include <k_syscall.h>
#include <process.h>
#include <k_debug.h>
#include <scheduler.h>
#include <thread.h>
#include <vmm.h>
#include <procmm.h>

#include <errno.h>
#include <strings.h>

#undef errno
extern int errno;


void  _exit(int rc)
{
  exit_process(current->proc, rc);

  current->state = THREAD_STATE_FINISHED;

  schedule();

  debug("ERROR! REACHED END OF EXIT SYSCALL!");
  for(;;);
}
KDEF_SYSCALL(exit, r)
{
  process_stack stack = init_pstack();

  _exit(stack[0]);

  return r;
}

int execve(char *name, char **argv, char **env)
{
  // Find the executable
  // Clear memory areas
  // Load new areas
  /* debug("EXECVE(%s, %x, %x)", name, argv, env); */
  fs_node_t *executable = vfs_find_node(name);
  uint32_t ex_size = executable->length;
  procmm_removeall(current->proc);
  load_elf(executable);
    current->r.eax = current->r.ebx = current->r.ecx = current->r.edx = 0;
    new_area(current->proc, USER_STACK_TOP, USER_STACK_TOP, MM_FLAG_WRITE | MM_FLAG_GROWSDOWN | MM_FLAG_ADDONUSE, MM_TYPE_STACK);
    current->r.useresp = current->r.ebp = USER_STACK_TOP;
    current->kernel_thread = current;
  errno = 0;
  return 0;
}
KDEF_SYSCALL(execve, r)
{
  thread_t *ct = current;
  process_stack stack = init_pstack();
  r->eax = execve((char *)stack[0], (char **)stack[1], (char **)stack[2]);
  r->ebx = errno;
  if(r->eax != -1)
  {
    current->r.eip = current->proc->mm.code_entry;
  }
  return r;
}

int fork()
{
  process_t *child = fork_process();
  thread_t *cth = list_entry(child->threads.next, thread_t, process_threads);
  cth->r.eax = 0;
  errno = 0;
  scheduler_insert(cth);
  return child->pid;
}
KDEF_SYSCALL(fork, r)
{
  r->eax = fork();
  r->ebx = errno;
  return r;
}

int getpid()
{
  errno = 0;
  return current->proc->pid;
}
KDEF_SYSCALL(getpid, r)
{
  r->eax = getpid();
  r->ebx = errno;
  return r;
}

int kill(int pid, int sig)
{
  errno = EINVAL;
  return -1;
}
KDEF_SYSCALL(kill, r)
{
  process_stack stack = init_pstack();
  r->eax = kill(stack[0], stack[1]);
  r->ebx = errno;
  return r;
}

int wait(int *status)
{
  errno = ECHILD;
  return -1;
}
KDEF_SYSCALL(wait, r)
{
  process_stack stack = init_pstack();
  r->eax = wait((int *)stack[0]);
  r->ebx = errno;
  return r;
}

int waitpid(int pid)
{
  process_t *proc = get_process(pid);

  while(proc->state != PROC_STATE_FINISHED)
  {
    scheduler_sleep(current, &proc->waiting);
    
    schedule();
  }

  int ret = proc->exit_code;
  free_process(proc);

  errno = 0;
  return ret;
}
KDEF_SYSCALL(waitpid, r)
{
  process_stack stack = init_pstack();
  r->eax = waitpid(stack[0]);
  r->ebx = errno;
  return r;
}

void yield()
{
  schedule();
  errno = 0;
}
KDEF_SYSCALL(yield, r)
{
  yield();
  return r;
}

