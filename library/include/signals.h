#pragma once

#include <stdint.h>

#ifndef __ASSEMBLER__

#define NUM_SIGNALS 64
#define SIG_IGNORE 0x0
#define SIG_DFL 0x1

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGFPE 8
#define SIGKILL 9
#define SIGSEGV 11
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTOP 17
#define SIGTSTP 18
#define SIGIO 23

void init_signals();
void raise(uint32_t signum);
uintptr_t *signal(uint32_t signum, uintptr_t *handler);

typedef void(*signal_handler_t)(uint32_t signum);

void _signal_event();

#endif
