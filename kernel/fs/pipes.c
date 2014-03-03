#include <vfs.h>
#include <lists.h>
#include <synch.h>
#include <scheduler.h>
#include <k_debug.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>


int32_t pipe_open(INODE ino, uint32_t flags)
{
  (void)flags;
  vfs_pipe_t *pipe = (vfs_pipe_t *)ino->data;
  if(ino->flags == PIPE_READ)
    pipe->readers = pipe->readers + 1;
  if(ino->flags == PIPE_WRITE)
    pipe->writers = pipe->writers + 1;
  return 0;
}
int32_t pipe_close(INODE ino)
{
  vfs_pipe_t *pipe = (vfs_pipe_t *)ino->data;
  if(ino->flags == PIPE_READ)
    pipe->readers = pipe->readers - 1;
  if(ino->flags == PIPE_WRITE)
    pipe->writers = pipe->writers - 1;
  if(!pipe->readers && !pipe->writers)
  {
    // Destroy pipe
  }
  return 0;
}
uint32_t pipe_read(INODE ino, void *buffer, uint32_t size, uint32_t offset)
{
  (void)offset;
  vfs_pipe_t *pipe = (vfs_pipe_t *)ino->data;
  uint32_t read = 0;
  while(read == 0)
  {
    spin_lock(&pipe->semaphore);
      if(!pipe->writers)
      {
        ((char *)buffer)[read++] = EOF;
        spin_unlock(&pipe->semaphore);
        return read;
      }
    while((pipe->write_pos - pipe->read_pos > 0) && read < size)
    {
      ((char *)buffer)[read] = pipe->buffer[pipe->read_pos % pipe->size];
      read++;
      pipe->read_pos++;
    }
    spin_unlock(&pipe->semaphore);
    scheduler_wake(&pipe->waiting);

    if(read == 0)
    {
      scheduler_sleep(current, &pipe->waiting);
      schedule();
    }
  }
  return read;
}
uint32_t pipe_write(INODE ino, void *buffer, uint32_t size, uint32_t offset)
{
  (void)offset;
  vfs_pipe_t *pipe = (vfs_pipe_t *)ino->data;
  uint32_t written = 0;
  while(written < size)
  {
    spin_lock(&pipe->semaphore);
    if(!pipe->readers)
    {
      while(written < size)
      {
        pipe->buffer[pipe->write_pos % pipe->size] = ((char *)buffer)[written];
        written++;
        pipe->write_pos++;
      }
    } else {
      while((pipe->write_pos - pipe->read_pos) < pipe->size && written < size)
      {
        pipe->buffer[pipe->write_pos % pipe->size] = ((char *)buffer)[written];
        written++;
        pipe->write_pos++;
      }
    }
    spin_unlock(&pipe->semaphore);
    scheduler_wake(&pipe->waiting);
    if(written < size)
    {
      scheduler_sleep(current, &pipe->waiting);
      schedule();
    }
  }
  return written;
}
int32_t pipe_stat(INODE node, struct stat *st)
{
  (void)node;
  memset(st, 0, sizeof(struct stat));
  st->st_mode = S_IFCHR;
  return 0;
}
int32_t pipe_isatty(INODE node)
{
  (void)node;
  return 1;
}


vfs_driver_t pipe_driver =
{
  pipe_open,
  pipe_close,
  pipe_read,
  pipe_write,
  0,
  0,
  pipe_stat,
  pipe_isatty,
  0,
  0,
  0,
  0
};

uint32_t new_pipe(uint32_t size, INODE *nodes)
{
  vfs_pipe_t *pipe = calloc(1, sizeof(vfs_pipe_t));
  pipe->buffer = malloc(size);
  pipe->size = size;
  spin_unlock(&pipe->semaphore);
  init_list(pipe->waiting);


  // Read inode
  nodes[0] = calloc(1, sizeof(vfs_node_t));
  strcpy(nodes[0]->name, "[pipe]");
  nodes[0]->d = &pipe_driver;
  nodes[0]->type = FS_PIPE;
  nodes[0]->data = pipe;
  nodes[0]->flags = PIPE_READ;

  // Write inode
  nodes[1] = calloc(1, sizeof(vfs_node_t));
  strcpy(nodes[1]->name, "[pipe]");
  nodes[1]->d = &pipe_driver;
  nodes[1]->type = FS_PIPE;
  nodes[1]->data = pipe;
  nodes[1]->flags = PIPE_WRITE;

  return 0;
}
