#pragma once

#include <stdint.h>

#define SYSCALL_YIELD 0x01
#define SYSCALL_PUTCH	0x02
#define SYSCALL_FORK	0x03
#define SYSCALL_GETPID	0x04
#define SYSCALL_EXECV	0x05
#define SYSCALL_PRINTF	0x06

#define DECL_SYSCALL0(name) \
	int _syscall_##name()
#define DECL_SYSCALL1(name, P1) \
	int _syscall_##name(P1)
#define DECL_SYSCALL2(name, P1, P2) \
	int _syscall_##name(P1, P2)
#define DECL_SYSCALL3(name, P1, P2, P3) \
	int _syscall_##name(P1, P2, P3)

#define DEF_SYSCALL0(name, num) \
	int _syscall_##name() \
	{ \
		int ret; \
		__asm__ volatile("int $0x80" : "=r" (ret) : "a" (num)); \
		return ret; \
	}
#define DEF_SYSCALL1(name, num, P1) \
	int _syscall_##name(P1 p1) \
	{ \
		int ret; \
		__asm__ volatile("int $0x80" : "=r" (ret) : "a" (num), "b" (p1)); \
		return ret; \
	}
#define DEF_SYSCALL2(name, num, P1, P2) \
	int _syscall_##name(P1 p1, P2 p2) \
	{ \
		int ret; \
		__asm__ volatile("int $0x80" : "=r" (ret) : "a" (num), "b" (p1), "c" (p2)); \
		return ret; \
	}
#define DEF_SYSCALL3(name, num, P1, P2, P3) \
	int _syscall_##name(P1 p1, P2 p2, P3 p3) \
	{ \
		int ret; \
		__asm__ volatile("int $0x80" : "=r" (ret) : "a" (num), "b" (p1), "c" (p2), "d" (p3)); \
		return ret; \
	}

DECL_SYSCALL1(putch, char);
DECL_SYSCALL1(printf, char *);
DECL_SYSCALL0(fork);
DECL_SYSCALL0(getpid);
DECL_SYSCALL3(execv, uintptr_t, char **, char **);

