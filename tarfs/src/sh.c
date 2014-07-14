#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#define BUF_SIZE 1024

int fg_pid = 0;

void _syscall_waitpid(uint32_t pid);

char buffer2[BUF_SIZE];

char *parse_command(char *buffer)
{
  char *p = buffer;
  char *p2 = buffer2;

  int i = 0;
  char env[64];
  while(*p)
  {
    if(p2 > &buffer2[BUF_SIZE])
      return 0;
    switch(*p)
    {
      case '$':
        p++;
        i = 0;
        while((*p != '\0') && isalnum((int)*p) && (i < 64))
        {
          env[i++] = *p++;
        }
        env[i] = '\0';

        char *var = getenv(env);
        if(var)
        {
          for(i = 0; i < (int)strlen(var); i++)
          {
            *p2 = var[i];
            p2++;
          }
          p2--;
        }
        break;
      case '\n':
        *p2 = '\0';
        break;
      default:
        *p2 = *p;
    }

    p++;
    p2++;
  }
  *p2 = '\0';
  return buffer2;
}

char **tokenize_command(char *line)
{
  char *command = strdup(line);
  int numtokens = 1;
  int i = 0;
  while(command[i])
  {
    if(command[i] == ' ')
    {
      numtokens++;
    }
    i++;
  }
  char **tokens = calloc(numtokens+1, sizeof(char *));
  tokens[0] = command;
  i = 0;
  int t = 1;
  while(command[i])
  {
    if(command[i] == ' ')
    {
      tokens[t++] = &command[i+1];
      command[i] = '\0';
    }
    i++;
  }
  tokens[t] = 0;

  return tokens;
}

void print_prompt()
{
  if(getenv("PS1"))
  {
    printf("%s", getenv("PS1"));
  } else {
    setenv("PS1","$ ",1);
    printf("$ ");
  }
}

int kill(int, int);
void signal_transfer(int signum)
{
  if(fg_pid)
    kill(fg_pid, signum);
}

int main(int argc, char **argv)
{
  (void)argc;
  char line[BUF_SIZE];

  signal(2, &signal_transfer);
  
  while(1)
  {
    print_prompt();
    fflush(stdout);

    fgets(line, BUF_SIZE, stdin);
    uint32_t len = strlen(line);

    if(len > 1)
    {
      line[len-1] = '\0';
      char *line2 = parse_command(line);
      char **tokens = tokenize_command(line2);

      if(strchr(line2, '='))
      {
        putenv(line2);
        continue;
      }

      fg_pid = fork();
      if(fg_pid)
      {
        _syscall_waitpid(fg_pid);
        fg_pid = 0;
      } else {
        if(execvp(tokens[0], tokens))
          printf("%s: command not found: %s\n", argv[0], tokens[0]);
        free(tokens);
        return -1;
      }
    }

  }
  return 0;
}
