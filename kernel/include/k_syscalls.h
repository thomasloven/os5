#pragma once

#include <syscalls.h>
#include <arch.h>

#define NUM_SYSCALLS 256

#ifndef __ASSEMBLER__

typedef registers_t *(*syscall_t)(registers_t *);

#define KDECL_SYSCALL(name) \
registers_t *k_syscall_##name(registers_t *)

#define KDEF_SYSCALL(name, r) \
registers_t *k_syscall_##name(registers_t *r)

#define KREG_SYSCALL(name, num) \
register_syscall(num, &k_syscall_##name)


void syscalls_init();
syscall_t register_syscall(uint32_t num, syscall_t handler);

KDECL_SYSCALL(putch);
KDECL_SYSCALL(getpid);
KDECL_SYSCALL(fork);
KDECL_SYSCALL(execv);

#endif

