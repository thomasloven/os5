#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int kill(pid_t pid, int sig);

int variable = 1;
void handler(int a)
{
  variable = 2;
  printf("Signal handler got %d\n", a);
  fflush(stdout);
}

int main()
{
  printf("Registering signal handler\n");
  signal(2, &handler);
  printf("Sending kill request\n");
  kill(getpid(), 2);
  printf("Variable=%d\n", variable);
  return 0;
}
