#pragma once
#include <signal.h>
#include <thread.h>


int signal_process(int pid, int signum);
sig_t switch_handler(int signum, sig_t handler);
void return_from_signal(registers_t *r);
thread_t *handle_signals(thread_t *th);
