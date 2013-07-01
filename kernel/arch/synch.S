; Bussy waiting for now...
[global spin_lock]

spin_lock:
  mov edx, [esp + 4]
  mov eax, 0
  mov ecx, 1
.retry:
  xor eax, eax
  lock cmpxchg [edx], cl
  jnz .retry

  ret
