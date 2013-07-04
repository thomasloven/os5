#include <vfs.h>
#include <lists.h>
#include <synch.h>
#include <scheduler.h>
#include <k_debug.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// One byte is wasted here...
// Or used as buffer, if you're an optimist
uint32_t pipe_unread_bytes(vfs_pipe_t *pipe)
{
  if(pipe->write_pos == pipe->read_pos)
    return 0;
  if(pipe->write_pos > pipe->read_pos)
    return pipe->write_pos - pipe->read_pos;
  else
    return (pipe->size - pipe->read_pos) + pipe->write_pos;
}
uint32_t pipe_unwritten_bytes(vfs_pipe_t *pipe)
{
  if(pipe->write_pos == pipe->read_pos)
    return pipe->size;
  if(pipe->write_pos > pipe->read_pos)
    return (pipe->size - pipe->write_pos) + pipe->read_pos -1;
  else
    return pipe->read_pos - pipe->write_pos -1;
}

uint32_t read_pipe(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  vfs_pipe_t *pipe = (vfs_pipe_t *)node->device;
  uint32_t read = 0;
  while(read == 0)
  {
    spin_lock(&pipe->semaphore);
    while(pipe_unread_bytes(pipe) > 0 && read < size)
    {
      buffer[read++] = pipe->buffer[pipe->read_pos++];
      if(pipe->read_pos == pipe->size)
        pipe->read_pos = 0;
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

uint32_t write_pipe(fs_node_t *node, uint32_t offset, uint32_t size, char *buffer)
{
  vfs_pipe_t *pipe = (vfs_pipe_t *)node->device;
  uint32_t written = 0;
  while(written < size)
  {
    spin_lock(&pipe->semaphore);
      while(pipe_unwritten_bytes(pipe) > 0 && written < size)
      {
        pipe->buffer[pipe->write_pos++] = buffer[written++];
        if(pipe->write_pos == pipe->size)
          pipe->write_pos = 0;
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

void open_pipe(fs_node_t *node, uint32_t flags)
{
  return;
}

void close_pipe(fs_node_t *node)
{
  return;
}

fs_node_t *new_pipe(uint32_t size)
{
  vfs_pipe_t *pipe = malloc(sizeof(vfs_pipe_t));
  pipe->buffer = malloc(size);
  pipe->size = size;
  pipe->read_pos = 0;
  pipe->write_pos = 0;
  pipe->users = 0;
  spin_unlock(&pipe->semaphore);
  init_list(pipe->waiting);


  fs_node_t *node = malloc(sizeof(fs_node_t));
  memset(node, 0, sizeof(fs_node_t));
  sprintf(node->name, "[pipe]");
  node->read = &read_pipe;
  node->write = &write_pipe;
  node->open = &open_pipe;
  node->close = &close_pipe;
  node->readdir = 0;
  node->finddir = 0;
  node->device = (void *)pipe;

  return node;
}
