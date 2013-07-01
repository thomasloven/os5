#include <vfs.h>
#include <k_syscall.h>
#include <syscall.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <k_debug.h>

#include <sys/stat.h>
#include <errno.h>

#undef errno
extern int errno;

int open(const char *name, int flags, int mode)
{
  // open as called by both kernel and usermode programs

  process_t *p = current->proc;

  // Find a free descriptor
  int i;
  int fd = -1;
  for(i=0; i < 256; i++)
  {
    if(!p->fd[i].node)
    {
      fd = i;
      break;
    }
  }
  if(fd == -1)
  {
    // No free descriptors
    errno = ENFILE;
    return fd;
  }

  // Open the file
  fs_node_t *node = vfs_find_node(name);
  vfs_open(node, mode);
  p->fd[fd].node = node;
  p->fd[fd].offset = 0;

  errno = 0;

  // Return the file descriptor index
  return fd;

}

KDEF_SYSCALL(open, r)
{

  process_stack stack = init_pstack();
  r->eax = open((char *)stack[0], stack[1], stack[2]);
  r->ebx = errno;
  return r;
}


int write(int file, char *ptr, int len)
{
  // Write called by both kernel and users

  ptr[len] = '\0';
  process_t *p = current->proc;

  fs_node_t *node = p->fd[file].node;

  errno = 0;

  return vfs_write(node, 0, len, (char *)ptr);
}


KDEF_SYSCALL(write, r)
{
  process_stack stack = init_pstack();
  r->eax = write(stack[0], (char *)stack[1], stack[2]);
  r->ebx = SYSCALL_OK;
  return r;
}

int fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return -1;
}

int close(int file)
{
  return -1;
}

int isatty(int file)
{
  return 1;
}

int lseek(int file, int ptr, int dir)
{
  return 0;
}

int read(int file, char *ptr, int len)
{
  return 0;
}
