#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -z $BUILDROOT ]; then
  BUILDROOT=$DIR/..
fi

. $BUILDROOT/util/common.sh

GRUBDIR=$TEMPDIR/grub-0.97-i386-pc

DISKSIZE="20M"
DISKFILE="image.img"

gototmp
  if [ ! -d $GRUBDIR ]; then
    download "grub-legacy" "ftp://alpha.gnu.org/gnu/grub" "grub-0.97-i386-pc.tar.gz"
    unzip "grub-0.97-i386-pc.tar.gz"
  fi

  type dito-generate >/dev/null 2>&1
  if [ $? != 0 ]; then
    gitclone "dito" "https://github.com/thomasloven/dito.git"
    pushd dito
      make || dropout
      make install-homebrew || dropout
    popd
  fi
popd

if [ "$1" = "force" ]; then
  rm -f $DISKFILE
fi

if [ ! -f $DISKFILE ]; then
  # Generate image and transfer stage2
  dito-generate $DISKFILE $DISKSIZE
  dito-format $DISKFILE:1 ext2 || dropout
  dito-mkdir ext2:$DISKFILE:1:/boot || dropout
  dito-mkdir ext2:$DISKFILE:1:/boot/grub || dropout
  dito-cp $GRUBDIR/boot/grub/stage2 ext2:$DISKFILE:1:/boot/grub/stage2 || dropout

  # Edit data in grub stages 1 and 1.5
    # Stage 1.5 address
  printf '\x20' | dd of=$GRUBDIR/boot/grub/stage1 bs=1 seek=67 count=1 conv=notrunc || dropout
    # Stage 1.5 segment
  printf '\x02' | dd of=$GRUBDIR/boot/grub/stage1 bs=1 seek=73 count=1 conv=notrunc || dropout

    # Stage 1.5 block count
  printf '\x12' | dd of=$GRUBDIR/boot/grub/e2fs_stage1_5 bs=1 seek=508 count=1 conv=notrunc || dropout
    # Stage 1.5 disk and partition
  printf '\x00\xff\xff\xff' | dd of=$GRUBDIR/boot/grub/e2fs_stage1_5 bs=1 seek=535 count=4 conv=notrunc || dropout

  # Transfer grub stages 1 and 1.5
  dd if=$GRUBDIR/boot/grub/stage1 of=$DISKFILE bs=1 count=445 conv=notrunc || dropout
  dd if=$GRUBDIR/boot/grub/e2fs_stage1_5 of=$DISKFILE bs=512 seek=1 conv=notrunc || dropout

  # Add Grub menu file
  dito-cp - ext2:$DISKFILE:1:/boot/grub/menu.lst << EOF
default 0
timeout 0

title os5
root (hd0,0)
kernel /boot/kernel
module /boot/tarfs
EOF

fi

# Copy kernel and tarfs
dito-cp $BUILDDIR/kernel/kernel ext2:$DISKFILE:1:/boot/kernel
dito-cp $BUILDDIR/tarfs.tar ext2:$DISKFILE:1:/boot/tarfs

