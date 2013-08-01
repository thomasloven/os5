# Bussy waiting for now...
.global spin_lock

spin_lock:
  mov 4(%esp), %edx
  mov $0, %eax
  mov $1, %ecx
.retry:
  xor %eax, %eax
  lock cmpxchg %cl, (%edx)
  jnz .retry

  ret
