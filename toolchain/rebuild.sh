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

  mkdir -p build-binutils
  pushd build-binutils
    ../binutils/configure --target=$TARGET --prefix=$PREFIX
    make -j
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

  mkdir -p build-gcc
  pushd build-gcc
    ../gcc/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c,c++
    make all-gcc -j
    make install-gcc
    make all-target-libgcc -j
    make install-target-libgcc
  popd
}

function prepare_newlib {
  if [ ! -d newlib ]; then
    git clone --depth 1 git://sourceware.org/git/newlib.git newlib
  fi

  # Building newlib requires special versions of automake and autoconf
  if [ ! -d /usr/local/Cellar/automake/1.12 ]; then
    curl -O http://ftp.gnu.org/gnu/automake/automake-1.12.tar.gz
    tar -zxf automake-1.12.tar.gz
    mkdir -p build-automake
    pushd build-automake
      ../automake-1.12/configure --prefix=/usr/local/Cellar/automake/1.12
      make all -j
      make install
    popd
  fi
  if [ ! -d /usr/local/Cellar/autoconf/2.64 ]; then
    curl -O http://ftp.gnu.org/gnu/autoconf/autoconf-2.64.tar.gz
    tar -zxf autoconf-2.64.tar.gz
    mkdir -p build-autoconf
    pushd build-autoconf
      ../autoconf-2.64/configure --prefix=/usr/local/Cellar/autoconf/2.64
      make all -j
      make install
    popd
  fi

  brew switch automake 1.12
  brew switch autoconf 2.64

  pushd newlib
    git reset HEAD .
    git checkout -- .
    git clean -f
    rm -rf newlib/libc/sys/myos
    git apply $PATCHDIR/newlib.patch
    pushd newlib/libc/sys
      cp -r $PATCHDIR/myos .
      autoconf
      pushd myos
        autoreconf
      popd
    popd
  popd

}


function rebuild_newlib {
  mkdir -p build-newlib
  pushd build-newlib
    rm -rf *
    ../newlib/configure --target=$TARGET --prefix=$PREFIX
    export CPPFLAGS_FOR_TARGET=-DKERNEL_MODE
    make -j
    make install
    mv $PREFIX/$TARGET/lib/libc.a $PREFIX/$TARGET/lib/libkernel.a
    rm -rf *
    ../newlib/configure --target=$TARGET --prefix=$PREFIX
    export CPPFLAGS_FOR_TARGET=
    make -j
    make install
  popd
}


export TARGET=i586-pc-myos
export PREFIX=/usr/local/Cellar/osdev/1.0

export PATCHDIR=`pwd`/toolchain

mkdir -p ~/osdev
pushd ~/osdev

rebuild_binutils
brew link osdev
rebuild_gcc
brew unlink osdev
brew link osdev
prepare_newlib
rebuild_newlib

