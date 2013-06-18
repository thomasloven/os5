#pragma once
#include <stdint.h>

#define SYSCALL_FORK 0x01
#define SYSCALL_GETPID 0x02
#define SYSCALL_PRINTF 0x03
#define SYSCALL_UNIMPL 0x04
#define SYSCALL_EXIT 0x05
#define SYSCALL_WAIT 0x06
#define SYSCALL_SBRK 0x07

#define SYSCALL_OPEN 0x10

#define SYSCALL_OK 0x00
#define ERROR_NOSYSCALL 0x01

#define DECL_SYSCALL0(name) \
  uint32_t _syscall_##name()
#define DECL_SYSCALL1(name, P1) \
  uint32_t _syscall_##name(P1)
#define DECL_SYSCALL2(name, P1, P2) \
  uint32_t _syscall_##name(P1, P2)
#define DECL_SYSCALL3(name, P1, P2, P3) \
  uint32_t _syscall_##name(P1, P2, P3)
#define DECL_SYSCALL1E(name, P1) \
  uint32_t _syscall_##name(P1, ...)


#ifndef __ASSEMBLER__

DECL_SYSCALL0(fork);
DECL_SYSCALL0(getpid);
DECL_SYSCALL1(exit, uint32_t);
DECL_SYSCALL1(wait, uint32_t);

DECL_SYSCALL1(sbrk, uint32_t);

DECL_SYSCALL1E(printf, char*);
DECL_SYSCALL0(unimpl);

uint32_t syscall_errno;

#else

#define DEF_SYSCALL(name, num) \
  _syscall_##name: \
  defsyscall num

%macro defsyscall 1
  mov eax, %1
  int 0x80
  mov [syscall_errno], ebx
  ret
%endmacro


#endif
