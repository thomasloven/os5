#include "syscalls.h"
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include <string.h>
#include <stdlib.h>

#undef errno
extern int errno;

#define NDEBUG
extern char **environ;

#ifndef KERNEL_MODE

char *__env[1] = { 0 };

void _init(uint32_t *args)
{
  /* _init_signal(); */

  int argc;
  char **argv;
  if(args)
  {
    argc = args[0];
    argv = (char **)args[1];
    environ = (char **)args[2];
  } else {
    argc = 0;
    argv = 0;
    environ = __env;
  }

  exit(main(argc, argv));
  for(;;);
}


void  _exit(int rc)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall _exit(%x)", rc);
#endif

  _syscall_exit(rc);
  for(;;);
}

int close(int file)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall close(%x)", file);
#endif

  int ret = _syscall_close(file);
  errno = syscall_errno;
  return ret;
}

int execve(const char *name, char *const argv[], char *env[])
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall execve(%s, %x, %x)", name, argv, env);
#endif
  /* errno = ENOMEM; */
  /* return -1; */
  int ret = _syscall_execve(name, argv, env);
  errno = syscall_errno;
  return ret;
}

int fork()
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall fork()");
#endif
  int ret = _syscall_fork();
  errno = syscall_errno;
  return ret;
}

int fstat(int file, struct stat *st)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall fstat(%x, %x)",file, st);
#endif
  /* st->st_mode = S_IFCHR; */
  /* return -1; */
  int ret = _syscall_fstat(file, st);
  errno = syscall_errno;
  return ret;
}

int getpid()
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall getpid()");
#endif

  int ret = _syscall_getpid();
  errno = syscall_errno;
  return ret;
}

int isatty(int file)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall isatty(%x)", file);
#endif
  /* return 1; */
  int ret = _syscall_isatty(file);
  errno = syscall_errno;
  return ret;
}

int kill(int pid, int sig)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall kill(%x, %x)", pid, sig);
#endif
  /* errno = EINVAL; */
  /* return -1; */
  int ret = _syscall_kill(pid, sig);
  errno = syscall_errno;
  return ret;
}

int link(char *old, char *new)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall link(%s, %s)", old, new);
#endif
  /* errno=EMLINK; */
  /* return -1; */
  int ret = _syscall_link(old, new);
  errno = syscall_errno;
  return ret;
}

int lseek(int file, int ptr, int dir)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall lseek(%x, %x, %x)", file, ptr, dir);
#endif
  /* return 0; */
  int ret = _syscall_lseek(file, ptr, dir);
  errno = syscall_errno;
  return ret;
}

int open(const char *name, int flags, int mode)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall open(%s %x %x)", name, flags, mode);
#endif
  int ret = _syscall_open(name, flags, mode);
  errno = syscall_errno;
  return ret;
}

int read(int file, char *ptr, int len)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall read(%x, %x, %x)", file, ptr, len);
#endif
  /* return 0; */
  int ret = _syscall_read(file, ptr, len);
  errno = syscall_errno;
  return ret;
}

void *sbrk(int incr)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall sbrk(%x)", incr);
#endif
  void *ret = (void *)_syscall_sbrk(incr);
  errno = syscall_errno;
  return ret;
}

int stat(const char *file, struct stat *st)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall stat(%s, %x)", file, st);
#endif
  /* st->st_mode = S_IFCHR; */
  /* return 0; */
  int ret = _syscall_stat(file, st);
  errno = syscall_errno;
  return ret;
}

clock_t times(struct tms *buf)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall times(%x)", buf);
#endif
  /* return -1; */
  clock_t ret = (clock_t)_syscall_times(buf);
  errno = syscall_errno;
  return ret;
}

int unlink(char *name)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall unlink(%s)", name);
#endif
  /* errno=ENOENT; */
  /* return -1; */
  int ret = _syscall_unlink(name);
  errno = syscall_errno;
  return ret;
}

int wait(int *status)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall wait(%x)", status);
#endif
  /* errno = ECHILD; */
  /* return -1; */
  int ret = _syscall_wait(status);
  errno = syscall_errno;
  return ret;
}

int write(int file, char *ptr, int len)
{
  ptr[len] = '\0';
#ifndef NDEBUG
  _syscall_printf("\n Syscall write()");
#endif
  int ret = _syscall_write(file, ptr, len);
  errno = syscall_errno;
  return ret;
}

sig_t signal(int signum, sig_t handler)
{
  sig_t ret = (sig_t)_syscall_signal(signum, handler);
  errno = syscall_errno;
  return ret;
}

#endif

int execvp(const char *file, char *const argv[])
{
  int i = 0;
  int addpath = 1;
  while(file[i])
  {
    if(file[i] == '/')
    {
      addpath = 0;
      break;
    }
    i++;
  }

  if(addpath)
  {
    char *path = strdup(getenv("PATH"));
    if(!path) path = "/usr/sbin:/bin";
    char *brk;
    char *p = strtok_r(path, ":", &brk);
    while(p)
    {
      char *fullpath = malloc(strlen(p) + strlen(file) + 1);
      strcpy(fullpath, p);
      strcat(fullpath, "/");
      strcat(fullpath, file);

      execve(fullpath, argv, environ);

      p = strtok_r(NULL, ":", &brk);
    }
  } else {
    execve(file, argv, environ);
  }

  return -1;
}

