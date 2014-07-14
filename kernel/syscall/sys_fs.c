#include <vfs.h>
#include <k_syscall.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <k_debug.h>
#include <procmm.h>

#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/dirent.h>
#include <string.h>
#include <fcntl.h>

#undef errno
extern int errno;

int close(int file)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]CLOSE(%x)\n", file);
  }
  process_t *p = current->proc;
  if(!p->fd[file]->ino)
  {
    errno = EBADF;
    return -1;
  }

  int retval = vfs_close(p->fd[file]->ino);
  vfs_free(p->fd[file]->ino);
  fd_put(p->fd[file]);
  p->fd[file] = 0;

  return retval;
}
KDEF_SYSCALL(close, r)
{
  process_stack stack = init_pstack();
  r->eax = close(stack[0]);
  r->edx = errno;
  return r;
}

int fstat(int file, struct stat *st)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]FSTAT(%d, %x)\n", file, st);
  }
  process_t *p = current->proc;
  INODE node = p->fd[file]->ino;
  if(!node)
  {
    errno = EBADF;
    return -1;
  }
  if(kernel_booted && procmm_check_address((uintptr_t)st) <= 1)
  {
    errno = EFAULT;
    return -1;
  }
  return vfs_stat(node, st);
}
KDEF_SYSCALL(fstat, r)
{
  process_stack stack = init_pstack();
  r->eax = fstat(stack[0], (struct stat *) stack[1]);
  r->edx = errno;
  return r;
}

int isatty(int file)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]ISATTY(%d)\n", file);
  }
  process_t *p = current->proc;
  INODE node = p->fd[file]->ino;
  if(!node)
    return 0;
  return vfs_isatty(node);
}
KDEF_SYSCALL(isatty, r)
{
  process_stack stack = init_pstack();
  r->eax = isatty(stack[0]);
  r->edx = errno;
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
  r->edx = errno;
  return r;
}

int lseek(int file, int ptr, int dir)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]LSEEK(%d, %x, %x)\n", file, ptr, dir);
  }
    debug("[info]LSEEK(%d, %x, %x)\n", file, ptr, dir);
  process_t *p = current->proc;
  INODE node = p->fd[file]->ino;
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
  if((int)p->fd[file]->offset + ptr < 0)
  {
    errno = EINVAL;
    return -1;
  }
  if((node->type & FS_PIPE) == FS_PIPE)
  {
    errno = ESPIPE;
    return -1;
  }
  
  if(dir == SEEK_SET) // 0
  {
    p->fd[file]->offset = ptr;
  }
  if(dir == SEEK_CUR) // 1
  {
    p->fd[file]->offset += ptr;
  }
  if(dir == SEEK_END) // 2
  {
    p->fd[file]->offset = node->length + ptr;
  }
  return p->fd[file]->offset;
}
KDEF_SYSCALL(lseek, r)
{
  process_stack stack = init_pstack();
  r->eax = lseek(stack[0], stack[1], stack[2]);
  r->edx = errno;
  return r;
}

int open(const char *name, int flags, ...)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]OPEN(%s, %x)\n", name, flags);
  }

  // Sanity check path address
  if(kernel_booted &&!procmm_check_address((uintptr_t)&name[0]))
  {
    errno = EFAULT;
    return -1;
  }

  // Find a free descriptor
  process_t *p = current->proc;
  int i;
  int fd = -1;
  for(i=0; i < NUM_FILEDES; i++)
  {
    if(p->fd[i])
      continue;
    fd = i;
    p->fd[fd] = calloc(1, sizeof(file_desc_t));
    fd_get(p->fd[fd]);
    break;
  }
  if(fd == -1)
  {
    // No free descriptors
    errno = EMFILE;
    return fd;
  }

  // Open the file
  char *n = canonicalize_path(name, 0);
  INODE node = vfs_namei(n);
  free(n);
  if(!node)
  {
    errno = ENOENT;
    fd_put(p->fd[fd]);
    p->fd[fd] = 0;
    return -1;
  }
  int retval = vfs_open(node, flags);
  if(retval < 0 )
  {
    vfs_free(node);
    fd_put(p->fd[fd]);
    p->fd[fd] = 0;
    return -1;
  }
  p->fd[fd]->ino = node;
  p->fd[fd]->offset = 0;
  p->fd[fd]->flags = flags;

  return fd;
}
KDEF_SYSCALL(open, r)
{
  process_stack stack = init_pstack();
  r->eax = open((const char *)stack[0], stack[1], stack[2]);
  r->edx = errno;
  return r;
}

int read(int file, char *ptr, int len)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]READ(%d, %x, %x)\n", file, ptr, len);
  }
  if(kernel_booted && (procmm_check_address((uintptr_t)ptr) <=1 ))
  {
    errno = EFAULT;
    return -1;
  }

  process_t *p = current->proc;
  INODE node = p->fd[file]->ino;
  if(!node)
  {
    errno = EBADF;
    return -1;
  }
  int ret = vfs_read(node, ptr, len, p->fd[file]->offset);
  p->fd[file]->offset += ret;
  return ret;
}
KDEF_SYSCALL(read, r)
{
  process_stack stack = init_pstack();
  r->eax = read(stack[0], (char *)stack[1], stack[2]);
  r->edx = errno;
  return r;
}

int stat(const char *file, struct stat *st)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]STAT(%s, %x)\n", file, st);
  }
  if(kernel_booted && !procmm_check_address((uintptr_t)&file[0]))
  {
    errno = EFAULT;
    return -1;
  }
  if(kernel_booted && procmm_check_address((uintptr_t)st) <= 1)
  {
    errno = EFAULT;
    return -1;
  }
  INODE node = vfs_namei(file);
  int retval = vfs_stat(node, st);
  vfs_free(node);
  return retval;
}
KDEF_SYSCALL(stat, r)
{
  process_stack stack = init_pstack();
  r->eax = stat((const char *)stack[0], (struct stat *)stack[1]);
  r->edx = errno;
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
  r->edx = errno;
  return r;
}

int write(int file, char *ptr, int len)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]WRITE(%d, %x, %x)\n", file, ptr, len);
  }
  if(kernel_booted && !procmm_check_address((uintptr_t)ptr))
  {
    errno = EFAULT;
    return -1;
  }

  /* ptr[len] = '\0'; */
  process_t *p = current->proc;
  INODE node = p->fd[file]->ino;
  if(!node)
  {
    errno = EBADF;
    return -1;
  }
  int ret =  vfs_write(node, ptr, len, p->fd[file]->offset);
  p->fd[file]->offset += ret;
  return ret;
}
KDEF_SYSCALL(write, r)
{
  process_stack stack = init_pstack();
  r->eax = write(stack[0], (char *)stack[1], stack[2]);
  r->edx = errno;
  return r;
}

struct dirent *readdir(DIR *dirp)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]READDIR(%x, %x)\n", dirp->fd, dirp->cur_entry+1);
  }
  dirp->cur_entry++;

  process_t *p = current->proc;
  INODE node = p->fd[dirp->fd]->ino;
  if(!node)
  {
    errno = EBADF;
    return 0;
  }
  
  dirent_t *de = 0;
  if(!(de = vfs_readdir(node, dirp->cur_entry)))
    return 0;
  struct dirent *d = calloc(1, sizeof(struct dirent));
  strcpy(d->name, de->name);
  d->ino = 0;

  vfs_free(de->ino);
  free(de);

  return d;
}
KDEF_SYSCALL(readdir, r)
{
  static DIR dirp;
  process_stack stack = init_pstack();
  dirp.fd = stack[0];
  dirp.cur_entry = stack[1]-1;
  struct dirent *de = readdir(&dirp);
  if(de)
  {
    memcpy((void *)stack[2], de, sizeof(struct dirent));
    free(de);
    r->eax = 0;
    return r;
  }
  free(de);
  r->eax = -1;
  return r;
}

int dup(int fildes)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]DUP(%x)\n", fildes);
  }
  process_t *p = current->proc;
  if(!p->fd[fildes] || fildes < 0)
  {
    errno = EBADF;
    return -1;
  }
  int i;
  int fd = -1;
  for(i=0; i < NUM_FILEDES; i++)
  {
    if(p->fd[i])
      continue;
    fd = i;
    p->fd[fd] = p->fd[fildes];
    fd_get(p->fd[fildes]);
    break;
  }
  if(fd == -1)
    errno = EMFILE;
  return fd;
}
KDEF_SYSCALL(dup, r)
{
  process_stack stack = init_pstack();
  r->eax = dup(stack[0]);
  r->edx = errno;
  return r;
}

int dup2(int fildes1, int fildes2)
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]DUP2(%x, %x)\n", fildes1, fildes2);
  }
  process_t *p = current->proc;
  if(!p->fd[fildes1] || fildes1 < 0 || fildes2 < 0)
  {
    errno = EBADF;
    return -1;
  }
  if(fildes1 >= NUM_FILEDES || fildes2 >= NUM_FILEDES)
  {
    errno = EMFILE;
    return -1;
  }

  if(fildes1 == fildes2)
    return fildes2;
  if(p->fd[fildes2])
  {
    close(fildes2);
  }
  p->fd[fildes2] = p->fd[fildes1];
  fd_get(p->fd[fildes1]);

  return fildes2;
}
KDEF_SYSCALL(dup2, r)
{
  process_stack stack = init_pstack();
  r->eax = dup2(stack[0], stack[1]);
  r->edx = errno;
  return r;
}

int pipe(int fildes[2])
{
  if(current->proc->flags & PROC_FLAG_DEBUG)
  {
    debug("[info]PIPE(%x)\n", fildes);
  }

  process_t *p = current->proc;

  // Find two free file descriptors
  int i;
  fildes[0] = -1;
  for(i = 0; i < NUM_FILEDES; i++)
  {
    if(p->fd[i])
      continue;
    fildes[0] = i;
    p->fd[fildes[0]] = calloc(1, sizeof(file_desc_t));
    fd_get(p->fd[fildes[0]]);
    break;
  }
  fildes[1] = -1;
  for(i = 0; i < NUM_FILEDES; i++)
  {
    if(p->fd[i])
      continue;
    fildes[1] = i;
    p->fd[fildes[1]] = calloc(1, sizeof(file_desc_t));
    fd_get(p->fd[fildes[1]]);
    break;
  }

  if(fildes[0] == -1 || fildes[1] == -1)
  {
    errno = EMFILE;
    return -1;
  }

  // Create pipe
  INODE tmp[2];
  new_pipe(512, tmp);

  // Set up and open file descriptors
  p->fd[fildes[0]]->ino = tmp[0];
  p->fd[fildes[0]]->flags = O_RDONLY;
  vfs_open(tmp[0], O_RDONLY);
  p->fd[fildes[1]]->ino = tmp[1];
  p->fd[fildes[1]]->flags = O_WRONLY;
  vfs_open(tmp[1], O_WRONLY);

  return 0;
}
KDEF_SYSCALL(pipe, r)
{
  process_stack stack = init_pstack();
  r->eax = pipe((int *)stack[0]);
  r->edx = errno;
  return r;
}
