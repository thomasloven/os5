#! /bin/sh

#
# Create patches with
#   git add .
#   git diff --cached --no-color > patchfile
#   git reset HEAD .
#
function rebuild_binutils {
  if [ ! -d binutils ]; then
    git clone --depth 1 git://sourceware.org/git/binutils.git binutils
  fi
  pushd binutils
    git reset HEAD .
    git checkout -- .
    git clean -f
    git apply $PATCHDIR/binutils.patch
  popd

  mkdir build-binutils
  pushd build-binutils
    ../binutils/configure --target=$TARGET --prefix=$PREFIX/1.0
    make
    make install
  popd
}

function rebuild_gcc {
  if [ ! -d gcc ]; then
    git clone --depth 1 git://gcc.gnu.org/git/gcc.git
  fi
  pushd gcc
    git reset HEAD .
    git checkout -- .
    git clean -f
    git apply $PATCHDIR/gcc.patch
  popd

  mkdir build-gcc
  pushd build-gcc
    ../gcc/configure --target=$TARGET --prefix=$PREFIX/1.0 --disable-nls --enable-languages=c,c++
    make all-gcc
    make install-gcc
  popd
}

function rebuild_newlib {
  if [ ! -d newlib ]; then
    git clone --depth 1 git://sourceware.org/git/newlib.git newlib
  fi

  # Building newlib requires special versions of automake and autoconf
  curl -O http://ftp.gnu.org/gnu/automake/automake-1.12.tar.gz
  curl -O http://ftp.gnu.org/gnu/autoconf/autoconf-2.64.tar.gz
  tar -zxf automake-1.12.tar.gz
  tar -zxf autoconf-2.64.tar.gz
  mkdir build-automake
  pushd build-automake
    ../automake-1.12/configure --prefix=/usr/local/Cellar/automake112
    make all
    make install
  popd
  mkdir build-autoconf
  pushd build-autoconf
    ../autoconf-2.64/configure --prefix=/usr/local/Cellar/autoconf264
    make all
    make install
  popd

  PATH=/usr/local/Cellar/automake112/bin:/usr/local/Cellar/autoconf264/bin:$PATH

  pushd newlib
    git reset HEAD .
    git checkout -- .
    git clean -f
    rm newlib/libc/sys/myos
    git apply $PATCHDIR/newlib.patch
    pushd newlib/libc/sys
      cp -r $PATCHDIR/myos .
      autoconf
      pushd myos
        autoreconf
      popd
    popd
  popd


  mkdir build-newlib
  pushd build-newlib
    ../newlib/configure --target=$TARGET --prefix=$PREFIX/1.0
    # Newlib handles cross compilers really badly, which is strange considering what most people use it for...
PATH=$PREFIX/1.0/$TARGET/bin:$PATH
    make
    make install
  popd
}

function rebuild_newlib_lite {
  pushd newlib/newlib/libc/sys
  PATH=/usr/local/Cellar/automake112/bin:/usr/local/Cellar/autoconf264/bin:$PATH
    cp -r $PATCHDIR/myos .
    autoconf
    pushd myos
      autoreconf
    popd
  popd
  mkdir build-newlib
  pushd build-newlib
    rm -rf *
    ../newlib/configure --target=$TARGET --prefix=$PREFIX/1.0
# PATH=$PREFIX/1.0/$TARGET/bin:$PATH
    make
    make install
  popd
}

-e

export TARGET=i586-pc-myos
export PREFIX=/usr/local/Cellar/osdev

export PATCHDIR=`pwd`/toolchain

mkdir ~/osdev
pushd ~/osdev

PATH=$PATH:$PREFIX/1.0/bin
# rebuild_binutils
# brew link osdev
# rebuild_gcc
rebuild_newlib_lite

