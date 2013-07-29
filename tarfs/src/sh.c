#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>

#define BUF_SIZE 1024

extern char **environ;

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

int execvp(const char *file, char *const argv[])
{
  int i = 0;
  int addpath = 1;
  while(file[i])
  {
    if(file[i] == '/')
    {
      addpath = 0;
      break;
    }
    i++;
  }

  if(addpath)
  {
    char *path = strdup(getenv("PATH"));
    if(!path) path = "/usr/sbin:/bin";
    char *brk;
    char *p = strtok_r(path, ":", &brk);
    while(p)
    {
      char *fullpath = malloc(strlen(p) + strlen(file) + 1);
      strcpy(fullpath, p);
      strcat(fullpath, "/");
      strcat(fullpath, file);

      execve(fullpath, argv, environ);

      p = strtok_r(NULL, ":", &brk);
    }
  }

  return -1;

}

int main(int argc, char **argv)
{
  char line[BUF_SIZE];
  
  while(1)
  {
    print_prompt();
    fflush(stdout);

    fgets(line, BUF_SIZE, stdin);
    uint32_t len = strlen(line);

    if(len > 1)
    {
      line[len-1] = '\0';
      char **tokens = tokenize_command(line);

      if(strchr(line, '='))
      {
        putenv(line);
        continue;
      }

      int pid = fork();
      if(pid)
      {
        _syscall_waitpid(pid);

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
