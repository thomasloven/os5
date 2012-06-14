#!/bin/bash

if [ "`uname -s`" = "Darwin" ]; then
	mydev='build/floppy.img'
	mpoint='/Volumes/osdev'
	mcmd='fuse-ext2'
	mcmd2='-o force'
fi

sudo mkdir $mpoint
sudo $mcmd $mydev $mpoint $mcmd2 >/dev/null

echo "   Copying"
sudo cp kernel/kernel $mpoint
sudo cp build/grub.conf $mpoint/boot/grub/grub.conf
sudo cp build/menu.lst $mpoint/boot/grub/menu.lst
echo "   Copied"

sudo umount -f $mpoint
echo "   Unmounted"
