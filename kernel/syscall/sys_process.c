#include <k_syscall.h>
#include <process.h>
#include <k_debug.h>
#include <scheduler.h>
#include <thread.h>
#include <vmm.h>
#include <procmm.h>
#include <elf.h>
#include <signals.h>

#include <errno.h>
#include <strings.h>
#include <string.h>
#include <malloc.h>
#include <signal.h>

#undef errno
extern int errno;


void  _exit(int rc)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]EXIT(%x)\n", rc);
    print_areas(current->proc);
  }

  int i;
  process_t *p = current->proc;
  for(i = 0; i < NUM_FILEDES; i++)
  {
    if(p->fd[i])
    {
      close(i);
    }
  }

  exit_process(current->proc, rc);

  current->state = THREAD_STATE_FINISHED;

  schedule();

  debug("[error]ERROR! REACHED END OF EXIT SYSCALL!\n");
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
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]EXECVE(%s, %x, %x)\n", name, argv, env);
  }
  // Find the executable
  INODE executable = vfs_namei(name);
  if(!executable)
  {
    debug("[error] Executable %s not found.\n", name);
    errno = ENOENT;
    return -1;
  }
  if(is_elf(executable) != 2)
  {
    errno = ENOEXEC;
    debug("[error] Tried to load unexecutable file.\n");
    return -1;
  }

  // Save environment in kernel space
  unsigned int envc = 0;
  char **temp_env;
  if(env)
  {
    while(env[envc++]);

    temp_env = calloc(envc, sizeof(char *));

    unsigned int i = 0;
    while(env[i])
    {
      temp_env[i] = strdup(env[i]);
      i++;
    }
    temp_env[envc-1] = 0;
  }

  // Save arguments in kernel space
  unsigned int argc = 0;
  char **temp_argv;
  if(argv)
  {
    while(argv[argc++]);

    temp_argv = calloc(argc, sizeof(char *));

    unsigned int i = 0;
    while(argv[i])
    {
      temp_argv[i] = strdup(argv[i]);
      i++;
    }
    temp_argv[argc-1] = 0;
  }

  if(current->proc->cmdline)
    free(current->proc->cmdline);
  current->proc->cmdline = strdup(name);

  // Clear all process memory areas
  procmm_removeall(current->proc);

  // Reset signal handlers
  memset(current->proc->signal_handler, 0, sizeof(sig_t)*NUM_SIGNALS);
  memset(current->proc->signal_blocked, 0, NUM_SIGNALS);
  init_list(current->proc->signal_queue);

  // Load executable
  load_elf(executable);

  // Reset thread registers and state
  current->r.eax = current->r.ebx = current->r.ecx = current->r.edx = 0;

  // Add an area for the process stack
  new_area(current->proc, USER_STACK_TOP, USER_STACK_TOP, MM_FLAG_WRITE | MM_FLAG_GROWSDOWN | MM_FLAG_ADDONUSE, MM_TYPE_STACK);
  current->kernel_thread = (registers_t *)current;
  uint32_t *pos = (uint32_t *)USER_STACK_TOP; // uint32_t since the stack should be word alligned

  // Restore environment
  if (env)
  {
    pos = pos - envc*sizeof(char *)/sizeof(uint32_t) - 1;
    env = (char **)pos;
    int i = 0;
    while(temp_env[i])
    {
      pos = pos - strlen(temp_env[i])/sizeof(uint32_t) - 2;
      memcpy(pos, temp_env[i], strlen(temp_env[i])+1);
      env[i] = (char *)pos;
      i++;
    }
    env[envc-1] = 0;
  }

  // Restore arguments
  if(argv)
  {
    pos = pos - argc*sizeof(char *)/sizeof(uint32_t) - 1;
    argv = (char **)pos;
    int i = 0;
    while(temp_argv[i])
    {
      pos = pos - strlen(temp_argv[i])/sizeof(uint32_t) - 2;
      memcpy(pos, temp_argv[i], strlen(temp_argv[i])+1);
      argv[i] = (char *)pos;
      i++;
    }
    argv[argc-1] = 0;
  }

  pos = pos - 3;
  pos[0] = (uint32_t)argc-1;
  pos[1] = (uint32_t)argv;
  pos[2] = (uint32_t)env;

  current->r.useresp = current->r.ebp = (uint32_t)pos;
  current->r.ecx = (uint32_t)pos;

  errno = 0;
  return 0;
}
KDEF_SYSCALL(execve, r)
{
  process_stack stack = init_pstack();
  r->eax = execve((char *)stack[0], (char **)stack[1], (char **)stack[2]);
  r->edx = errno;
  if(r->eax != (uint32_t)-1)
  {
    current->r.eip = current->proc->mm.code_entry;
  }
  return r;
}

int fork()
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]FORK()\n");
  }
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
  r->edx = errno;
  return r;
}

int getpid()
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]GETPID()\n");
  }
  errno = 0;
  return current->proc->pid;
}
KDEF_SYSCALL(getpid, r)
{
  r->eax = getpid();
  r->edx = errno;
  return r;
}

int kill(int pid, int sig)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]KILL(%d, %d)\n", pid, sig);
  }
  /* debug("KILL(%d, %d)", pid, sig); */
  if(pid <= 0)
  {
    errno = ESRCH;
    return -1;
  }
  if(sig > NUM_SIGNALS)
  {
    errno = EINVAL;
    return -1;
  }
  if(!get_process(pid))
  {
    errno = ESRCH;
    return -1;
  }

  signal_process(pid, sig);

  return 0;
}
KDEF_SYSCALL(kill, r)
{
  process_stack stack = init_pstack();
  r->eax = kill(stack[0], stack[1]);
  r->edx = errno;
  return r;
}

int wait(int *status)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]WAIT(%x)\n", status);
  }
  errno = ECHILD;
  return -1;
}
KDEF_SYSCALL(wait, r)
{
  process_stack stack = init_pstack();
  r->eax = wait((int *)stack[0]);
  r->edx = errno;
  return r;
}

int waitpid(int pid)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]WAITPID(%d)\n", pid);
  }
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
  r->edx = errno;
  return r;
}

void yield()
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]YIELD()\n");
  }
  schedule();
  errno = 0;
}
KDEF_SYSCALL(yield, r)
{
  yield();
  return r;
}

sig_t signal(int sig, sig_t handler)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]SIGNAL(%d, %x)\n", sig, handler);
  }
  /* debug("SIGNAL(%d %x)", sig, handler); */
  if(sig > NUM_SIGNALS)
  {
    errno = EINVAL;
    return (sig_t)-1;
  }
  if(sig == SIGKILL || sig == SIGSTOP)
  {
    if( handler != 0)
    {
      errno = EINVAL;
      return (sig_t)-1;
    }
  }
  process_t *p = current->proc;
  sig_t old = p->signal_handler[sig];
  p->signal_handler[sig] = handler;
  return old;
}
KDEF_SYSCALL(signal, r)
{
  process_stack stack = init_pstack();
  r->eax = (uint32_t)signal(stack[0], (sig_t)stack[1]);
  r->edx = errno;
  return r;
}

KDEF_SYSCALL(process_debug, r)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]PROCDB()\n");
  }
  debug("[info]Starting debug\n");
  current->proc->flags |= PROC_FLAG_DEBUG;
  debug("[info]Process: %x \n", current->proc->pid);
  return r;
}

KDEF_SYSCALL(thread_fork, r)
{
  // example: thread_fork(stack, function, argument);
  process_stack stack = init_pstack();
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]thread_fork(%x, %x, %x)\n", stack[0], stack[1], stack[2]);
  }
  thread_t *th = new_thread((void (*)(void))stack[1], 1);
  append_to_list(current->proc->threads, th->process_threads);
  th->proc = current->proc;
  uint32_t *stk = (uint32_t *)stack[0];
  *--stk = stack[2];
  *--stk = SIGNAL_RETURN_ADDRESS;
  th->r.useresp = th->r.ebp = (uint32_t)stk;
  r->eax = th->tid;
  return r;
}

