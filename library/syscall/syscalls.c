#include <syscall.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <errno.h>

#define NDEBUG

#undef errno
extern int errno;

int _exit(int rc)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall _exit()");
#endif
  return _syscall_exit(rc);
}

int close(int file)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall close(%x)", file);
#endif
  return -1;
}

char *__env[1] = { 0 };
char **environ = __env;

int execve(char *name, char **argv, char **env)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall execve()");
#endif
  errno = ENOMEM;
  return -1;
}

int fork()
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall fork()");
#endif
  return _syscall_fork();
}

int fstat(int file, struct stat *st)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall fstat()");
#endif
  st->st_mode = S_IFCHR;
  return -1;
}

int getpid()
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall getpid()");
#endif
  return _syscall_getpid();
}

int isatty(int file)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall isatty()");
#endif
  return 1;
}

int kill(int pid, int sig)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall kill()");
#endif
  errno = EINVAL;
  return -1;
}

int link(char *old, char *new)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall link()");
#endif
  errno=EMLINK;
  return -1;
}

int lseek(int file, int ptr, int dir)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall lseek()");
#endif
  return 0;
}

int open(const char *name, int flags, int mode)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall open(%x %x %x)", name, flags, mode);
#endif
  return _syscall_open(name, flags, mode);
}

int read(int file, char *ptr, int len)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall read()");
#endif
  return 0;
}

void *sbrk(int incr)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall sbrk(%x)", incr);
#endif
  return (void *)_syscall_sbrk(incr);
}

int stat(const char *file, struct stat *st)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall stat()");
#endif
  st->st_mode = S_IFCHR;
  return 0;
}

clock_t times(struct tms *buf)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall times()");
#endif
  return -1;
}

int unlink(char *name)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall unlink()");
#endif
  errno=ENOENT;
  return -1;
}

int wait(int *status)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall wait()");
#endif
  errno = ECHILD;
  return -1;
}

int write(int file, char *ptr, int len)
{
  ptr[len] = '\0';
#ifndef NDEBUG
  _syscall_printf("\n Syscall write()");
#endif
  len = _syscall_write(file, ptr, len);
  return len;
}
