#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ -z $BUILDROOT ]; then
  BUILDROOT=$DIR/..
fi

. $BUILDROOT/util/common.sh

TARGET=i586-pc-myos
PREFIX=/usr/local/Cellar/osdev/1.0

binutilsv=binutils-2.24
gccv=gcc-4.8.2

function rebuild_binutils() {
  pushd $TEMPDIR >/dev/null

    ### Download and extract binutils
    #
    echo -e "${CLR_NO} Downloading ${CLR_BLUE}${binutilsv}${CLR_NO}"
    if [ ! -d $binutilsv ]; then
      download "Binutils" "ftp://ftp.gnu.org/gnu/binutils/" "${binutilsv}.tar.gz" 2>${TEMPDIR}/error.log || dropout
      echo -e "extracting"
      unzip "${binutilsv}.tar.gz" >/dev/null 2>${TEMPDIR}/error.log || dropout
    fi
    echo -en "${CLR_NO} Downloading ${CLR_BLUE}${binutilsv}${CLR_NO} ... "
    echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    ### Patch binutils
    #
    echo -en "${CLR_NO} Patching ${CLR_BLUE}binutils${CLR_NO} ... "
    if [ ! -f $binutilsv/.patched ]; then
      dopatch "Binutils" $binutilsv $BUILDROOT/toolchain/binutils.patch >/dev/null 2>${TEMPDIR}/error.log || dropout
      touch $binutilsv/.patched
    fi
    echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    ### Create or empty build directory
    #
    mkdir -p build-binutils
    pushd build-binutils >/dev/null
      rm -rf *

      ### ./configure
      #
      echo -en "${CLR_NO} Configuring ${CLR_BLUE}binutils${CLR_NO} ... "
      ../${binutilsv}/configure --disable-werror --target=$TARGET --prefix=$PREFIX >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make
      #
      echo -en "${CLR_NO} Building ${CLR_BLUE}binutils${CLR_NO} ... "
      make -j >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make install
      #
      echo -en "${CLR_NO} Installing ${CLR_BLUE}binutils${CLR_NO} ... "
      make install >/dev/null 2>${TEMPDIR}/error.log || dropout
      rm -r $PREFIX/share/locale
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    popd >/dev/null
  popd >/dev/null

  ### Install homebrew links
  #
  brew unlink osdev
  brew link osdev
}

function rebuild_gcc() {
  pushd $TEMPDIR >/dev/null

    ### Download and extract gcc
    #
    echo -e "${CLR_NO} Downloading ${CLR_BLUE}${gccv}${CLR_NO}"
    if [ ! -d $gccv ]; then
      download "gcc" "ftp://ftp.gnu.org/gnu/gcc/${gccv}/" "${gccv}.tar.gz" 2>${TEMPDIR}/error.log || dropout
      echo -e "extracting"
      unzip "${gccv}.tar.gz" >/dev/null 2>${TEMPDIR}/error.log || dropout
    fi
    echo -en "${CLR_NO} Downloading ${CLR_BLUE}${gccv}${CLR_NO} ... "
    echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    ### Patch gcc
    #
    echo -en "${CLR_NO} Patching ${CLR_BLUE}gcc${CLR_NO} ... "
    if [ ! -f $gccv/.patched ]; then
      dopatch "gcc" $gccv $BUILDROOT/toolchain/gcc.patch >/dev/null 2>${TEMPDIR}/error.log || dropout
      touch $gccv/.patched
    fi
    echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    ### Create or empty build directory
    #
    mkdir -p build-gcc
    pushd build-gcc >/dev/null
      rm -rf *

      ### ./configure
      #
      echo -en "${CLR_NO} Configuring ${CLR_BLUE}gcc${CLR_NO} ... "
      ../${gccv}/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c,c++ >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make gcc
      #
      echo -en "${CLR_NO} Building ${CLR_BLUE}gcc${CLR_NO} ... "
      make all-gcc -j >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make install gcc
      #
      echo -en "${CLR_NO} Installing ${CLR_BLUE}binutils${CLR_NO} ... "
      make install-gcc >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make libgcc
      #
      echo -en "${CLR_NO} Building ${CLR_BLUE}libgcc${CLR_NO} ... "
      make all-target-libgcc >/dev/null -j 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make install libgcc
      #
      echo -en "${CLR_NO} Installing ${CLR_BLUE}libgcc${CLR_NO} ... "
      make install-target-libgcc >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    popd >/dev/null
  popd >/dev/null

  ### Install homebrew links
  #
  brew unlink osdev >/dev/null
  brew link osdev >/dev/null
}

rm ${TEMPDIR}/error.log

type brew >/dev/null 2>&1
if [ $? != 0 ]; then
  echo -e "${CLR_RED} Homebrew was not found!${CLR_NO}"
  echo "This build tool relies on homebrew for osx."
  echo "If you're on a mac, I recommend you look it up"
  echo "http://brew.sh"
  echo ""
  echo "If you're not on a mac, I can't help you."
  echo "I appologize for the inconvenience."
  dropout
fi

echo -en "${CLR_NO} Checking for ${CLR_BLUE}binutils${CLR_NO} ... "
type ${TARGET}-ld >/dev/null 2>&1
if [ $? != 0 ]; then
  echo -e "${CLR_YELLOW}[Missing]${CLR_NO}"
  echo -e "${CLR_BLUE} Building binutils${CLR_NO}"
  rebuild_binutils
else
  echo -e "${CLR_GREEN}[OK]${CLR_NO}"
fi

echo -en "${CLR_NO} Checking for ${CLR_BLUE}gcc${CLR_NO} ... "
type ${TARGET}-gcc >/dev/null 2>&1
if [ $? != 0 ]; then
  echo -e "${CLR_YELLOW}[Missing]${CLR_NO}"
  echo -e "${CLR_BLUE} Building binutils${CLR_NO}"
  rebuild_gcc
else
  echo -e "${CLR_GREEN}[OK]${CLR_NO}"
fi
