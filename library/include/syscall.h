#pragma once

#define SYSCALL_FORK 0x01
#define SYSCALL_GETPID 0x02
#define SYSCALL_PRINTF 0x03
#define SYSCALL_UNIMPL 0x04
#define SYSCALL_EXIT 0x05
#define SYSCALL_WAIT 0x06
#define SYSCALL_SBRK 0x07

#define SYSCALL_OK 0x00
#define ERROR_NOSYSCALL 0x01

#ifndef __ASSEMBLER__
#include <stdint.h>

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
