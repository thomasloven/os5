#!/bin/bash

if tmux info | grep $TTY ; then
	tmux split-window 'bochs-term -f build/bochsrc.txt'
else
	bochs-term -f build/bochsrc.txt
fi
