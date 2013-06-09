#pragma once

#ifdef __ASSEMBLER__

#else

typedef uintptr_t semaphore_t;

void spin_lock(semaphore_t *lock);

#define spin_unlock(addr) (*(addr)=0)

#endif
