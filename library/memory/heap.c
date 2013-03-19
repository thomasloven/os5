#include <heap.h>
#include <stdint.h>
#include <ctype.h>
#include <synch.h>
#include <syscall.h>
#include <memory.h>

#define HEAP_START 0x123
uintptr_t heap_top = 0;
uintptr_t heap_start = 0;
chunk_t *heap_first = 0;
semaphore_t heap_sem;

void expand_heap(uintptr_t start, uint32_t size)
{
  if(start == 0)
  {
    heap_start = _syscall_sbrk(size);
    heap_top = heap_start + size;
  } else {
    heap_top = heap_start + _syscall_sbrk(size);
  }
}

void contract_heap(chunk_t *c)
{
  c = 0;
}

void split_chunk(chunk_t *c, uint32_t size)
{
  if((c->size - size) > sizeof(chunk_t))
  {
    chunk_t *new = (chunk_t *)((uintptr_t)c + size);
    new->prev = c;
    new->next = c->next;
    new->size = c->size - size;
    new->allocated = FALSE;

    c->next = new;
    c->size = size;
  }
}

void glue_chunk(chunk_t *c)
{
  if(c->next)
    if(!c->next->allocated)
    {
      c->size = c->size + c->next->size;
      c->next = c->next->next;
      if(c->next) c->next->prev = c;
    }

  if(c->prev)
    if(!c->prev->allocated)
    {
      c->prev->size = c->prev->size + c->size;
      c->prev->next = c->next;
      if(c->next) c->next->prev = c->prev;
      c = c->prev;
    }

  if(!c->next)
    contract_heap(c);
}

void free(void *a)
{

  spin_lock(&heap_sem);
    chunk_head(a)->allocated = FALSE;
    glue_chunk(chunk_head(a));
  spin_unlock(&heap_sem);
}

void *malloc(uint32_t size)
{
  size += sizeof(chunk_t);
  chunk_t *cur_chunk = heap_first;
  chunk_t *prev_chunk = 0;

  spin_lock(&heap_sem);
    while(cur_chunk)
    {
      if(cur_chunk->allocated == FALSE && cur_chunk->size >= size)
      {
        split_chunk(cur_chunk, size);
        cur_chunk->allocated = TRUE;
      spin_unlock(&heap_sem);
        return chunk_data(cur_chunk);
      }
      prev_chunk = cur_chunk;
      cur_chunk = cur_chunk->next;
    }

    if(prev_chunk)
    {
      cur_chunk = (chunk_t *)((uint32_t)prev_chunk + prev_chunk->size);
    } else {
      heap_first = cur_chunk = 0;
    }

    expand_heap((uintptr_t)cur_chunk, size);
    if(cur_chunk == 0)
      cur_chunk = (chunk_t *)heap_start;
    memset(cur_chunk, 0, sizeof(chunk_t));
    cur_chunk->prev = prev_chunk;
    cur_chunk->next = 0;
    cur_chunk->size = size;
    cur_chunk->allocated = TRUE;
    if(prev_chunk) prev_chunk->next = cur_chunk;
  spin_unlock(&heap_sem);

  return chunk_data(cur_chunk);
}

void *calloc(uint32_t size)
{
  void *c = malloc(size);
  memset(c, 0, size);
  return c;
}
