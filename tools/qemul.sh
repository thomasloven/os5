#!/bin/bash

if tmux info | grep $TTY ; then
  tmux split-window -h 'qemu-system-i386 -kernel build/kernel -curses -monitor telnet:localhost:4444,server -s -S'
  tmux select-pane -L
  tmux split-window -v 'i386-elf-gdb'
  tmux select-pane -U
  sleep 0.1
  counter=10
  until [[ $counter -lt 1 ]]; do
    timestart=$(date +%s)
    telnet localhost 4444
    timeend=$(date +%s)
    timediff=$(echo "$timeend - $timestart" | bc)
    echo $timediff
    if [[ $timediff -gt 1 ]]; then
      echo "DONE!"
      counter=0
    else
      echo "Go again"
      counter=$counter-1
      sleep 1
    fi
  done
  tmux kill-pane -a
else
  qemu-system-i386 -kernel build/kernel -display curses -monitor stdio -s -S
fi
