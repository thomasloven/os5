#include <stdio.h>

int main(int argc, const char *argv[])
{
  (void)argc;
  (void)argv;

  char *string = (char *)0x12345678;

  printf("Causing page fault!\n");
  sprintf(string, "Page fault!");
  printf("Page fault caused.\n");

  return 0;
}
