#include <vfs.h>
#include <k_syscall.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <k_debug.h>

#include <sys/stat.h>
#include <errno.h>

#undef errno
extern int errno;

int close(int file)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]CLOSE(%x)\n", file);
  }
  process_t *p = current->proc;
  if(!p->fd[file].node)
  {
    errno = EBADF;
    return -1;
  }

  vfs_close(p->fd[file].node);
  p->fd[file].node = 0;
  errno = 0;
  return 0;
}
KDEF_SYSCALL(close, r)
{
  process_stack stack = init_pstack();
  r->eax = close(stack[0]);
  r->ebx = errno;
  return r;
}

int fstat(int file, struct stat *st)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]FSTAT(%d, %x)\n", file, st);
  }
  process_t *p = current->proc;
  fs_node_t *node = p->fd[file].node;
  if(!node)
  {
    errno = EBADF;
    return -1;
  }
  if(procmm_check_address(st) <= 1)
  {
    errno = EFAULT;
    return -1;
  }
  st->st_dev = 0;
  st->st_ino = node->inode;
  st->st_mode = node->mode;
  st->st_nlink = 0;
  st->st_uid = 0;
  st->st_gid = 0;
  st->st_rdev = 0;
  st->st_size = node->length;
  st->st_atime = 0;
  st->st_mtime = 0;
  st->st_ctime = 0;
  return 0;
}
KDEF_SYSCALL(fstat, r)
{
  process_stack stack = init_pstack();
  r->eax = fstat(stack[0], (struct stat *) stack[1]);
  r->ebx = errno;
  return r;
}

int isatty(int file)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]ISATTY(%d)\n", file);
  }
  errno = 0;
  process_t *p = current->proc;
  fs_node_t *node = p->fd[file].node;
  if(!node)
    return 0;
  return (node->flags & VFS_FLAG_ISATTY);
}
KDEF_SYSCALL(isatty, r)
{
  process_stack stack = init_pstack();
  r->eax = isatty(stack[0]);
  r->ebx = errno;
  return r;
}

int link(char *old, char *new)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]LINK(%s, %s)\n", old, new);
  }
  errno = EMLINK;
  return -1;
}
KDEF_SYSCALL(link, r)
{
  process_stack stack = init_pstack();
  r->eax = link((char *)stack[0], (char *)stack[1]);
  r->ebx = errno;
  return r;
}

int lseek(int file, int ptr, int dir)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]LSEEK(%d, %x, %x)\n", file, ptr, dir);
  }
  process_t *p = current->proc;
  fs_node_t *node = p->fd[file].node;
  if(!node)
  {
    errno = EBADF;
    return -1;
    // ERROR!
  }
  if(dir != 0 && dir != 1 && dir != 2)
  {
    errno = EINVAL;
    return -1;
  }
  if((int)p->fd[file].offset + ptr < 0)
  {
    errno = EINVAL;
    return -1;
  }
  if(node->flags & VFS_FLAG_PIPE)
  {
    errno = ESPIPE;
    return -1;
  }
  
  if(dir == 0) // SEEK_SET
  {
    p->fd[file].offset = ptr;
  }
  if(dir == 1) // SEEK_CUR
  {
    p->fd[file].offset += ptr;
  }
  if(dir == 2) // SEEK_END
  {
    p->fd[file].offset = node->length + ptr;
  }
  errno = 0;
  return p->fd[file].offset;
}
KDEF_SYSCALL(lseek, r)
{
  process_stack stack = init_pstack();
  r->eax = lseek(stack[0], stack[1], stack[2]);
  r->ebx = errno;
  return r;
}

int open(const char *name, int flags, int mode)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]OPEN(%s, %x, %x)\n", name, flags, mode);
  }
  if(kernel_booted &&!procmm_check_address(&name[0]))
  {
    errno = EFAULT;
    return -1;
  }
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
    errno = EMFILE;
    return fd;
  }

  // Open the file
  fs_node_t *node = vfs_find_node(name);
  if(!node)
  {
    errno = ENOENT;
    return -1;
  }
  errno = 0;
  vfs_open(node, flags);
  if(errno)
    return -1;
  p->fd[fd].node = node;
  p->fd[fd].offset = 0;
  p->fd[fd].flags = flags;

  return fd;
}
KDEF_SYSCALL(open, r)
{
  process_stack stack = init_pstack();
  r->eax = open((const char *)stack[0], stack[1], stack[2]);
  r->ebx = errno;
  return r;
}

int read(int file, char *ptr, int len)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]READ(%d, %x, %x)\n", file, ptr, len);
  }
  process_t *p = current->proc;
  fs_node_t *node = p->fd[file].node;
  if(!node)
  {
    errno = EBADF;
    return -1;
  }
  if(procmm_check_address(ptr) <=1 )
  {
    errno = EFAULT;
    return -1;
  }
  errno = 0;
  int ret = vfs_read(node, p->fd[file].offset, len, ptr);
  p->fd[file].offset += ret;
  return ret;
}
KDEF_SYSCALL(read, r)
{
  process_stack stack = init_pstack();
  r->eax = read(stack[0], (char *)stack[1], stack[2]);
  r->ebx = errno;
  return r;
}

int stat(const char *file, struct stat *st)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]STAT(%s, %x)\n", file, st);
  }
  if(kernel_booted &&!procmm_check_address(&file[0]))
  {
    errno = EFAULT;
    return -1;
  }
  if(procmm_check_address(st) <= 1)
  {
    errno = EFAULT;
    return -1;
  }
  st->st_mode = S_IFCHR;
  errno = 0;
  return 0;
}
KDEF_SYSCALL(stat, r)
{
  process_stack stack = init_pstack();
  r->eax = stat((const char *)stack[0], (struct stat *)stack[1]);
  r->ebx = errno;
  return r;
}

int unlink(char *name)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]UNLINK(%s)\n", name);
  }
  errno = ENOENT;
  return -1;
}
KDEF_SYSCALL(unlink, r)
{
  process_stack stack = init_pstack();
  r->eax = unlink((char *)stack[0]);
  r->ebx = errno;
  return r;
}

int write(int file, char *ptr, int len)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]WRITE(%d, %x, %x)\n", file, ptr, len);
  }
  // Write called by both kernel and users

  if(kernel_booted && !procmm_check_address(ptr))
  {
    errno = EFAULT;
    return -1;
  }

  ptr[len] = '\0';
  process_t *p = current->proc;

  fs_node_t *node = p->fd[file].node;
  if(!node)
  {
    errno = EBADF;
    return -1;
  }

  errno = 0;

  int ret =  vfs_write(node, p->fd[file].offset, len, (char *)ptr);
  p->fd[file].offset += ret;
  return ret;
}
KDEF_SYSCALL(write, r)
{
  process_stack stack = init_pstack();
  r->eax = write(stack[0], (char *)stack[1], stack[2]);
  r->ebx = errno;
  return r;
}
