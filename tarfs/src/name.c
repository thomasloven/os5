#include <stdio.h>
#include <string.h>

int main()
{
  char name[128];
  printf("Hi! What's your name?\n");
  fgets(name, 128, stdin);
  int len = strlen(name);
  name[len-1] = '\0';
  printf("Hello, %s.", name);
  return 0;
}
