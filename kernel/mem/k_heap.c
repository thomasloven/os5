#include <common.h>
#include <k_heap.h>
#include <stdint.h>
#include <ctype.h>
#include <vmm.h>
#include <pmm.h>
#include <memory.h>
#include <synch.h>
#include <k_debug.h>

#include <stdlib.h>
#include <errno.h>

uintptr_t heap_top = KERNEL_HEAP_START;
chunk_t *heap_first = 0;
semaphore_t heap_sem;

void expand_heap(uintptr_t start, uint32_t size)
{
  while(start + size > heap_top)
  {
    vmm_page_set(heap_top, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_WRITE));
    memset(heap_top, 0, PAGE_SIZE);
    heap_top += PAGE_SIZE;
    assert(heap_top < KERNEL_HEAP_END);
  }
}

void contract_heap(chunk_t *c)
{
  assert(!c->next);

  if(c == heap_first)
    heap_first = 0;
  else
    c->prev->next = 0;

  while(heap_top - PAGE_SIZE >= (uintptr_t)c)
  {
    pmm_free_page(vmm_page_get(heap_top) & PAGE_MASK);
    vmm_page_set(heap_top,0);
    heap_top -= PAGE_SIZE;
  }
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

void kfree(void *a)
{
  free(a);
  return;
  assert((uint32_t)a >= KERNEL_HEAP_START && (uint32_t)a < KERNEL_HEAP_END);

  spin_lock(&heap_sem);
    chunk_head(a)->allocated = FALSE;
    glue_chunk(chunk_head(a));
  spin_unlock(&heap_sem);
}

void *kmalloc(uint32_t size)
{
  return malloc(size);
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
      heap_first = cur_chunk = (chunk_t *)KERNEL_HEAP_START;
    }

    expand_heap((uintptr_t)cur_chunk, size);
    memset(cur_chunk, 0, sizeof(chunk_t));
    cur_chunk->prev = prev_chunk;
    cur_chunk->next = 0;
    cur_chunk->size = size;
    cur_chunk->allocated = TRUE;
    if(prev_chunk) prev_chunk->next = cur_chunk;
  spin_unlock(&heap_sem);

  return chunk_data(cur_chunk);
}

void *kcalloc(uint32_t size)
{
  void *c = kmalloc(size);
  memset(c, 0, size);
  return c;
}

void *kvalloc(uint32_t size)
{
  void *ret = valloc(size);
  return valloc(size);
  void *pos = kmalloc(size + PAGE_SIZE);
  split_chunk(chunk_head(pos), PAGE_SIZE - ((uint32_t) pos % PAGE_SIZE));

  chunk_t *new = chunk_head(pos)->next;
  split_chunk(new, size + sizeof(chunk_t));
  new->allocated = TRUE;

  kfree(pos);

  return chunk_data(new);
}

