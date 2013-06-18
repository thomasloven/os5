#include <vfs.h>
#include <k_syscall.h>
#include <syscall.h>
#include <arch.h>
#include <thread.h>
#include <process.h>

KDEF_SYSCALL(open, r)
{
// int open(const char *name, int flags, int mode)

  process_stack stack = init_pstack();
  process_t *p = current->proc;

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
    r->eax = fd;
    return r;
  }
  fs_node_t *node = vfs_find_node(stack[0]);
  vfs_open(node, stack[2]);
  p->fd[fd].node = node;
  p->fd[fd].offset = 0;
  r->eax = fd;
  return r;
  
}
