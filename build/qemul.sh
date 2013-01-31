#!/bin/bash

if tmux info | grep $TTY ; then
	tmux split-window -h 'qemu-system-i386 -kernel kernel/kernel -curses -monitor telnet:localhost:4444,server -s -S'
	tmux select-pane -L
	sleep 0.1
	telnet localhost 4444
else
	qemu-system-i386 -kernel kernel/kernel -display curses -monitor stdio -s -S
fi
