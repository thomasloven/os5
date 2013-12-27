#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

TEMPDIR=~/osdev/

if [ -z $BUILDDIR ]; then
  BUILDDIR=$BUILDROOT/build
fi

function gototmp() {
  mkdir -p $TEMPDIR
  pushd $TEMPDIR
}

function dropout() {
  echo "SOMETHING FAILED!"
  exit 1
}

# Download name url file
function download() {
  echo "Downloading $1"
  if [ ! -f "$3" ]; then
    curl -# -O "$2/$3"
    echo " Done"
  else
    echo " Already downloaded"
  fi
}

# Clone name git
function gitclone() {
  echo "Cloning $1"
  if [ ! -d "$1" ]; then
    git clone --depth=1 $2
    echo " Done"
  else
    echo " Already downloaded"
  fi
}

# Extract file
function unzip() {
  echo "Extracting $1"
  tar -xf $1
}

