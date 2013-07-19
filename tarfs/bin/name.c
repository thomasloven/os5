#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  printf("Hi! What's your name?\n");
  char name[128];
  fgets(name, 128, stdin);
  int len = strlen(name);
  name[len-1] = '\0';
  printf("Hello, %s.", name);
  return 0;
}
