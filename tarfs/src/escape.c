#include <stdio.h>

int main(int argc, const char *argv[])
{
  (void)argc;
  (void)argv;
  printf("Clearing screen \033[2J\n");
  printf("Right:\033[5Chere\n");
  printf("Left:     .\033[5Dhere\n");
  printf("Down:\033[2BUp:\033[1ABack\n");
  printf("Failed code \033[25;34;12x end\n");
  printf("\033[0;3HPosition\n");
  printf("\033[6;0fPosition2\n");
  printf("1234567890\033[3Dc\033[K\n");

  int i,j;
  for(i = 0; i < 8; i++)
  {
    printf("\033[0m");
    for(j = 0; j < 8; j++)
    {
      printf("\033[3%d;4%dmX",i,j);
    }
    printf("\033[0;1m");
    for(j = 0; j < 8; j++)
    {
      printf("\033[3%d;4%dmX",i,j);
    }
    printf("\033[0;7m");
    for(j = 0; j < 8; j++)
    {
      printf("\033[3%d;4%dmX",i,j);
    }
    printf("\033[0;1;7m");
    for(j = 0; j < 8; j++)
    {
      printf("\033[3%d;4%dmX",i,j);
    }
    printf("\n");
  }
  return 0;
}
