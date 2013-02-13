#!/bin/bash

if [ "`uname -s`" = "Darwin" ]; then
  mydev='build/floppy.img'
  mpoint='floppy'
  mcmd='fuse-ext2'
  mcmd2='-o force'
fi

cp build/floppy2.img build/floppy.img
if [ ! -d "$mpoint" ]; then
  mkdir $mpoint
fi
$mcmd $mydev $mpoint $mcmd2 >/dev/null

while [ ! -d $mpoint/boot ]; do
  echo "Waiting..."
  sleep 1
done
echo "   Copying"
cp kernel/kernel $mpoint
cp build/grub.conf $mpoint/boot/grub/grub.conf
cp build/menu.lst $mpoint/boot/grub/menu.lst
echo "   Copied"

umount $mpoint
echo "   Unmounted"
