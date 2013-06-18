#include <syscall.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <errno.h>

#undef errno
extern int errno;

int _exit(int rc)
{
  _syscall_printf("\n Syscall _exit()");
  return _syscall_exit(rc);
}

int close(int file)
{
  _syscall_printf("\n Syscall close()");
  return -1;
}

char *__env[1] = { 0 };
char **environ = __env;

int execve(char *name, char **argv, char **env)
{
  _syscall_printf("\n Syscall execve()");
  errno = ENOMEM;
  return -1;
}

int fork()
{
  _syscall_printf("\n Syscall fork()");
  return _syscall_fork();
}

int fstat(int file, struct stat *st)
{
  _syscall_printf("\n Syscall fstat()");
  st->st_mode = S_IFCHR;
  return -1;
}

int getpid()
{
  _syscall_printf("\n Syscall getpid()");
  return _syscall_getpid();
}

int isatty(int file)
{
  _syscall_printf("\n Syscall isatty()");
  return 1;
}

int kill(int pid, int sig)
{
  _syscall_printf("\n Syscall kill()");
  errno = EINVAL;
  return -1;
}

int link(char *old, char *new)
{
  _syscall_printf("\n Syscall link()");
  errno=EMLINK;
  return -1;
}

int lseek(int file, int ptr, int dir)
{
  _syscall_printf("\n Syscall lseek()");
  return 0;
}

int open(const char *name, int flags, int mode)
{
  _syscall_printf("\n Syscall open(%x %x %x)", name, flags, mode);
  return _syscall_open(name, flags, mode);
}

int read(int file, char *ptr, int len)
{
  _syscall_printf("\n Syscall read()");
  return 0;
}

void *sbrk(int incr)
{
  _syscall_printf("\n Syscall sbrk(%x)", incr);
  return (void *)_syscall_sbrk(incr);
}

int stat(const char *file, struct stat *st)
{
  _syscall_printf("\n Syscall stat()");
  st->st_mode = S_IFCHR;
  return 0;
}

clock_t times(struct tms *buf)
{
  _syscall_printf("\n Syscall times()");
  return -1;
}

int unlink(char *name)
{
  _syscall_printf("\n Syscall unlink()");
  errno=ENOENT;
  return -1;
}

int wait(int *status)
{
  _syscall_printf("\n Syscall wait()");
  errno = ECHILD;
  return -1;
}

int write(int file, char *ptr, int len)
{
  _syscall_printf("\n Syscall write()");
  _syscall_printf(ptr);
  return len;
}
