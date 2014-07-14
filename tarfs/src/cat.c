#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, const char *argv[])
{
  int ret = 0;
  int i;
  for(i = 1; i < argc; i++)
  {
    int fd = open(argv[i], O_RDONLY);
    if(!fd)
    {
      fprintf(stderr, "%s: %s: no such file or directory\n", argv[0], argv[i]);
      ret = 1;
      continue;
    } else {
      struct stat st;
      fstat(fd, &st);
      if(S_ISDIR(st.st_mode))
      {
        fprintf(stderr, "%s: %s: is a directory\n", argv[0], argv[i]);
        ret = 1;
        continue;
      } else {
        while(1)
        {
          char buf[1024];
          int r = read(fd, buf, 124);
          if(!r) break;
          fwrite(buf, 1, r, stdout);
          fflush(stdout);
        }
      }
    }
  }
  return ret;
}
