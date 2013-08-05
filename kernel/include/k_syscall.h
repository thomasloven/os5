#pragma once
#include <arch.h>

#include <sys/stat.h>
#include <sys/times.h>
#include <signal.h>

#define SYSCALL_EXIT 0x1
#define SYSCALL_CLOSE 0x2
#define SYSCALL_EXECVE 0x3
#define SYSCALL_FORK 0x4
#define SYSCALL_FSTAT 0x5
#define SYSCALL_GETPID 0x6
#define SYSCALL_ISATTY 0x7
#define SYSCALL_KILL 0x8
#define SYSCALL_LINK 0x9
#define SYSCALL_LSEEK 0xa
#define SYSCALL_OPEN 0xb
#define SYSCALL_READ 0xc
#define SYSCALL_SBRK 0xd
#define SYSCALL_STAT 0xe
#define SYSCALL_TIMES 0xf
#define SYSCALL_UNLINK 0x10
#define SYSCALL_WAIT 0x11
#define SYSCALL_WRITE 0x12

#define SYSCALL_PRINTF 0x13
#define SYSCALL_WAITPID 0x14
#define SYSCALL_YIELD 0x15

#define SYSCALL_SIGNAL 0x16

#define SYSCALL_PDBG 0x17


#define SYSCALL_OK 0x00
#define ERROR_NOSYSCALL 0x01

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

// sys_system.c
clock_t times(struct tms *buf);
KDECL_SYSCALL(times);

#endif
