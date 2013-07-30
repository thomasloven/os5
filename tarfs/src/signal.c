#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main()
{
  printf("Registering signal handler (not)\n");
  printf("Sending kill request\n");
  kill(getpid(), 2);
  return 0;
}
