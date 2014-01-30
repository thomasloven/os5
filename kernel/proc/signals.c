#include <signals.h>
#include <thread.h>
#include <process.h>
#include <vmm.h>
#include <scheduler.h>
#include <stdlib.h>
#include <k_debug.h>
#include <stdio.h>

#include <signal.h>
#include <unistd.h>

void sighandler_ignore(int num)
{
  (void)num;
  return;
}
void sighandler_terminate(int num)
{
  // Terminate process
  fprintf(stderr, "Process %x terminated by signal %x\n", \
      current->proc->pid, num);
  _exit(num);
  return;
}
void sighandler_coredump(int num)
{
  // Core dump and terminate
  (void)num;
  fprintf(stderr, "Process %x terminated by signal %x\n", \
      current->proc->pid, num);
  fprintf(stderr, "Should make a core dump...\n");
  _exit(num);
  return;
}
void sighandler_stop(int num)
{
  // Stop process
  (void)num;
  return;
}
void sighandler_continue(int num)
{
  // Continue stopped process
  (void)num;
  return;
}


sig_t default_sign[] = {
  0, 
  sighandler_terminate, // 1: SIGHUP
  sighandler_terminate, // 2: SIGING
  sighandler_coredump, // 3: SIGQUIT
  sighandler_coredump, // 4: SIGILL
  sighandler_coredump, // 5: SIGTRAP
  sighandler_coredump, // 6: SIGABRT
  sighandler_coredump, // 7: SIGEMT
  sighandler_coredump, // 8: SIGFPE
  sighandler_terminate, // 9: SIGKILL
  sighandler_coredump, // 10: SIGBUS
  sighandler_coredump, // 11: SIGSEGV
  sighandler_coredump, // 12: SIGSYS
  sighandler_terminate, // 13: SIGPIPE
  sighandler_terminate, // 14: SIGALRM
  sighandler_terminate, // 15: SIGTERM
  sighandler_terminate, // 16: SIGUSR1
  sighandler_terminate, // 17: SIGUSR2
  sighandler_ignore, // 18: SIGCHILD
  sighandler_ignore, // 19: SIGPWR
  sighandler_ignore, // 20: SIGWINCH
  sighandler_ignore, // 21: SIGURG
  sighandler_ignore, // 22: SIGPOLL
  sighandler_stop, // 23: SIGSTOP
  sighandler_stop, // 24: SIGTSTP
  sighandler_continue, // 25: SIGCONT
  sighandler_stop, // 26: SIGTTIN
  sighandler_stop, // 27: SIGTTOU
  sighandler_terminate, // 28: SIGVTALRM
  sighandler_terminate, // 29: SIGPROF
  sighandler_coredump, // 30: SIGXCPU
  sighandler_coredump, // 31: SIGXFSZ
  sighandler_ignore, // 32: SIGWAITING
};

int signal_process(int pid, int signum)
{
  process_t *p = get_process(pid);
  if(!p)
    return -1;
  if(p->state == PROC_STATE_FINISHED)
    return -1;

  signal_t *signal = malloc(sizeof(signal_t));
  signal->sig = signum;
  signal->sender = current->proc->pid;
  init_list(signal->queue);
  append_to_list(p->signal_queue, signal->queue);
  if(p == current->proc)
  {
    // Handle signals immediately
    handle_signals(current);
  }
  return 0;
}

sig_t switch_handler(int signum, sig_t handler)
{
  if(signum == SIGKILL)
  {
    // Never replace signal 9
    return SIG_DFL;
  }
  sig_t old = current->proc->signal_handler[signum];
  current->proc->signal_handler[signum] = handler;
  return old;
}

void return_from_signal(registers_t *r)
{
  (void)r;
  thread_t *th = current;
  scheduler_wake(&th->waiting);
  th->state = THREAD_STATE_FINISHED;
  scheduler_remove(th);

  schedule();
}

thread_t *handle_signals(thread_t *th)
{
  signal_t *signal = 0;
  list_t *i;
  for_each_in_list(&th->proc->signal_queue, i)
  {
    signal = list_entry(i, signal_t, queue);
    if(th->proc->signal_blocked[signal->sig])
      continue; // The signal is blocked
    break;
  }
  if(signal)
  {
    if(th->proc->signal_handler[signal->sig] == SIG_IGN)
    {
      remove_from_list(signal->queue);
      free(signal);
    } else if(th->proc->signal_handler[signal->sig] == SIG_DFL)
    {
      sig_t handler = default_sign[signal->sig];
      int signum = signal->sig;
      remove_from_list(signal->queue);
      free(signal);
      handler(signum);
    } else 
    {
      sig_t handler = th->proc->signal_handler[signal->sig];
      thread_t *h = new_thread((void (*)(void))handler, 1);

      append_to_list(th->proc->threads, h->process_threads);
      h->proc = th->proc;
      uint32_t *stack = (uint32_t *)th->r.useresp;
      *--stack = signal->sig;
      *--stack = SIGNAL_RETURN_ADDRESS;
      h->r.useresp = h->r.ebp = (uint32_t) stack;
      remove_from_list(signal->queue);
      free(signal);
      
      scheduler_remove(h);
      scheduler_sleep(th, &h->waiting);
      scheduler_cheat(h);
      schedule();
    }
  }

  return th;
}
