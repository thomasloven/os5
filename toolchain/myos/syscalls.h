#pragma once


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
#include <stdint.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <signal.h>

int syscall_errno;

#define DECL_SYSCALL0(name) \
  int _syscall_##name()
#define DECL_SYSCALL1(name, P1) \
  int _syscall_##name(P1)
#define DECL_SYSCALL2(name, P1, P2) \
  int _syscall_##name(P1, P2)
#define DECL_SYSCALL3(name, P1, P2, P3) \
  int _syscall_##name(P1, P2, P3)
#define DECL_SYSCALL1E(name, P1) \
  int _syscall_##name(P1, ...)


DECL_SYSCALL1(exit, int);
DECL_SYSCALL1(close, int);
DECL_SYSCALL3(execve, const char *, char *const*, char **);
DECL_SYSCALL0(fork);
DECL_SYSCALL2(fstat, int, struct stat *);
DECL_SYSCALL0(getpid);
DECL_SYSCALL1(isatty, int);
DECL_SYSCALL2(kill, int, int);
DECL_SYSCALL2(link, char *, char *);
DECL_SYSCALL3(lseek, int, int, int);
DECL_SYSCALL3(open, const char *, int, int);
DECL_SYSCALL3(read, int, char *, int);
DECL_SYSCALL1(sbrk, int);
DECL_SYSCALL2(stat, const char *, struct stat *);
DECL_SYSCALL1(times, struct tms *);
DECL_SYSCALL1(unlink, char *);
DECL_SYSCALL1(wait, int *);
DECL_SYSCALL3(write, int, char *, int);

DECL_SYSCALL1E(printf, char *);
DECL_SYSCALL1(waitpid, int);
DECL_SYSCALL0(yield);

DECL_SYSCALL2(signal, int, sighandler_t);

DECL_SYSCALL0(pdbg);


#else

#define DEF_SYSCALL(name, num) \
  _syscall_##name: \
  defsyscall num

.macro defsyscall num
  mov %ebx,(syscall_temp)
  mov $\num, %eax
  int $0x80
  mov %ebx, (syscall_errno)
  mov (syscall_temp), %ebx
  ret
.endm

#endif
