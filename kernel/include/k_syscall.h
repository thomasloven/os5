#pragma once
#include <arch.h>

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
  (uintptr_t *)(r->useresp + 0x4)


registers_t *syscall_handler(registers_t *r);
syscall_t register_syscall(uint32_t num, syscall_t handler);
void syscall_init();

KDECL_SYSCALL(printf);
KDECL_SYSCALL(fork);
KDECL_SYSCALL(getpid);
KDECL_SYSCALL(exit);
KDECL_SYSCALL(wait);
KDECL_SYSCALL(sbrk);

KDECL_SYSCALL(open);
KDECL_SYSCALL(write);

#endif
