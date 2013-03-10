#pragma once

#define SYSCALL_YIELD 0x01
#define SYSCALL_PUTCH 0x02
#define SYSCALL_FORK 0x03
#define SYSCALL_GETPID 0x04
#define SYSCALL_EXEXV 0x05
#define SYSCALL_PRINTF 0x06
#define SYSCALL_EXIT 0x07
#define SYSCALL_WAITPID 0x08

#define DECL_SYSCALL0(name) \
  int __cdecl _syscall_##name()
#define DECL_SYSCALL1(name, P1) \
  int __cdecl _syscall_##name(P1)
#define DECL_SYSCALL2(name, P1, P2) \
  int __cdecl _syscall_##name(P1, ...)
#define DECL_SYSCALL3(name, P1, P2, P3) \
  int __cdecl _syscall_##name(P1, P2, P3)

#define DEF_SYSCALL0(name, num) \
  int __cdecl _syscall_##name() \
  { \
    int ret; \
    __asm__ volatile("int $0x80" : "=r" (ret) : "a" (num)); \
    return ret; \
  }

#define DEF_SYSCALL1(name, num, P1) \
  int __cdecl _syscall_##name(P1 p1) \
  { \
    int ret; \
    __asm__ volatile("int $0x80" : "=r" (ret) : "a" (num)); \
    return ret; \
  }

#define DEF_SYSCALL2(name, num, P1, P2) \
  int __cdecl _syscall_##name(P1 p1, ...) \
  { \
    int ret; \
    __asm__ volatile("int $0x80" : "=r" (ret) : "a" (num)); \
    return ret; \
  }

DECL_SYSCALL0(fork);
DECL_SYSCALL2(printf, char *, char *);
