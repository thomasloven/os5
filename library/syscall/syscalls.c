#include <syscall.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <errno.h>

#undef errno
extern int errno;

#define NDEBUG

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

  return -1;
}

char *__env[1] = { 0 };
char **environ = __env;

int execve(char *name, char **argv, char **env)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall execve(%s, %x, %x)", name, argv, env);
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
  _syscall_printf("\n Syscall fstat(%x, %x)",file, st);
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
  _syscall_printf("\n Syscall isatty(%x)", file);
#endif
  return 1;
}

int kill(int pid, int sig)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall kill(%x, %x)", pid, sig);
#endif
  errno = EINVAL;
  return -1;
}

int link(char *old, char *new)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall link(%s, %s)", old, new);
#endif
  errno=EMLINK;
  return -1;
}

int lseek(int file, int ptr, int dir)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall lseek(%x, %x, %x)", file, ptr, dir);
#endif
  return 0;
}

int open(const char *name, int flags, int mode)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall open(%s %x %x)", name, flags, mode);
#endif
  return _syscall_open(name, flags, mode);
}

int read(int file, char *ptr, int len)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall read(%x, %x, %x)", file, ptr, len);
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
  _syscall_printf("\n Syscall stat(%s, %x)", file, st);
#endif
  st->st_mode = S_IFCHR;
  return 0;
}

clock_t times(struct tms *buf)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall times(%x)", buf);
#endif
  return -1;
}

int unlink(char *name)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall unlink(%s)", name);
#endif
  errno=ENOENT;
  return -1;
}

int wait(int *status)
{
#ifndef NDEBUG
  _syscall_printf("\n Syscall wait(%x)", status);
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
