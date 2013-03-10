#pragma once
#include <arch.h>
#include <syscall.h>

#define NUM_SYSCALLS 256

#ifndef __ASSEMBLER__

typedef registers_t *(*syscall_handler_t)(registers_t *);

#define KDECL_SYSCALL(name) \
  registers_t *k_syscall_##name(registers_t *)

#define KDEF_SYSCALL(name, r) \
  registers_t *k_syscall_##name(registers_t *r)

#define KREG_SYSCALL(name, num) \
  register_syscall(num, &k_syscall_##name)

void syscalls_init();
syscall_handler_t register_syscall(uint32_t num, syscall_handler_t handler);
registers_t *syscall_handler(registers_t *r);

#endif
