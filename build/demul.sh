#!/bin/bash

if tmux info | grep $TTY; then
	tmux split-window -h 'bochs -f build/bochsrc.txt'
else
	bochs -f build/bochsrc.txt
fi
