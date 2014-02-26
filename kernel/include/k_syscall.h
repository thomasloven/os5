#pragma once
#include <arch.h>

#include <sys/stat.h>
#include <sys/times.h>
#include <signal.h>
#include <sys/dirent.h>
#include <syscalls.h>


#ifndef __ASSEMBLER__

#define NUM_SYSCALLS 256

typedef registers_t *(*syscall_t)(registers_t *);

#define KDECL_SYSCALL(name) \
  registers_t *k_syscall_##name(registers_t *)

#define KDEF_SYSCALL(name, r) \
  registers_t *k_syscall_##name(registers_t *r)

#define KREG_SYSCALL(name, num) \
  register_syscall(num, &k_syscall_##name);

typedef uintptr_t* process_stack;
#define init_pstack() \
  (r->cs & 0x3)? \
  (uintptr_t *)(r->useresp + 0x4): \
  (uintptr_t *)(&r->ss)

#define is_kernel() \
  (!(r->cs & 0x3))


registers_t *syscall_handler(registers_t *r);
syscall_t register_syscall(uint32_t num, syscall_t handler);
void syscall_init();

KDECL_SYSCALL(printf);

// sys_process.c
void _exit();
KDECL_SYSCALL(exit);
int execve(char *name, char **argv, char **env);
KDECL_SYSCALL(execve);
int fork();
KDECL_SYSCALL(fork);
int getpid();
KDECL_SYSCALL(getpid);
int kill(int pid, int sig);
KDECL_SYSCALL(kill);
int wait(int *status);
KDECL_SYSCALL(wait);
int waitpid(int pid);
KDECL_SYSCALL(waitpid);
void yield();
KDECL_SYSCALL(yield);
sig_t signal(int sig, sig_t handler);
KDECL_SYSCALL(signal);
KDECL_SYSCALL(process_debug);
KDECL_SYSCALL(thread_fork);

// sys_mem.c
void *sbrk(int incr);
KDECL_SYSCALL(sbrk);

// sys_fs.c
int close(int file);
KDECL_SYSCALL(close);
int fstat(int file, struct stat *st);
KDECL_SYSCALL(fstat);
int isatty(int file);
KDECL_SYSCALL(isatty);
int link(char *old, char *new);
KDECL_SYSCALL(link);
int lseek(int file, int ptr, int dir);
KDECL_SYSCALL(lseek);
int open(const char *name, int flags, int mode);
KDECL_SYSCALL(open);
int read(int file, char *ptr, int len);
KDECL_SYSCALL(read);
int stat(const char *file, struct stat *st);
KDECL_SYSCALL(stat);
int unlink(char *name);
KDECL_SYSCALL(unlink);
int write(int file, char *ptr, int len);
KDECL_SYSCALL(write);
struct dirent *readdir(DIR *dirp);
KDECL_SYSCALL(readdir);
int dup(int fildes);
KDECL_SYSCALL(dup);
int dup2(int fildes1, int fildes2);
KDECL_SYSCALL(dup2);
int pipe(int fildes[2]);
KDECL_SYSCALL(pipe);

// sys_system.c
clock_t times(struct tms *buf);
KDECL_SYSCALL(times);

KDECL_SYSCALL(vidmem);

#endif
