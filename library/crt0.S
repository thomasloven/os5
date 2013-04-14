
[global _start]
[extern _init]
[global _signal_event]
[extern _on_event]

_start:
  call _init
  jmp $

_signal_event:
  push eax
  call _on_event
  jmp $
  
