#include <k_syscall.h>
#include <process.h>
#include <k_debug.h>
#include <scheduler.h>
#include <thread.h>
#include <vmm.h>
#include <procmm.h>
#include <elf.h>

#include <errno.h>
#include <strings.h>
#include <string.h>
#include <malloc.h>

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
  if(!executable)
  {
    errno = ENOENT;
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
    temp_env[envc] = 0;
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
    argv[argc] = 0;
  }

  // Clear all process memory areas
  procmm_removeall(current->proc);

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
      memcpy(pos, temp_argv[i], strlen(temp_argv[i]));
      argv[i] = (char *)pos;
      i++;
    }
    argv[argc] = 0;
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
  r->ebx = errno;
  if(r->eax != (uint32_t)-1)
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

