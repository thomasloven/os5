#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -z $TARGET ]; then
  TARGET=i586-pc-myos
fi
if [ -z $PREFIX ]; then
  PREFIX=/usr/local/Cellar/osdev/1.0
fi

TEMPDIR=~/osdev/

CLR_NO="\\033[0m"
CLR_RED="\\033[31m"
CLR_GREEN="\\033[32m"
CLR_YELLOW="\\033[33m"
CLR_BLUE="\\033[36m"

if [ -z $BUILDDIR ]; then
  BUILDDIR=$BUILDROOT/build
fi

function gototmp() {
  mkdir -p $TEMPDIR
  pushd $TEMPDIR
}

function dropout() {
  echo -e "${CLR_RED}SOMETHING FAILED!${CLR_NO}"
  if [ -f ${TEMPDIR}/error.log ]; then
    cat ${TEMPDIR}/error.log
  fi
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

# Patch files
function dopatch() {
  echo "Applying patch to $1"
  pushd $2
    patch -p1 < $3
  popd
}

