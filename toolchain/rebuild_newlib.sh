#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ -z $BUILDROOT ]; then
  BUILDROOT=$DIR/..
fi

. $BUILDROOT/util/common.sh

TARGET=i586-pc-myos
PREFIX=/usr/local/Cellar/osdev/1.0

newlibv=newlib-2.1.0

function prepare_automake() {
  pushd $TEMPDIR >/dev/null

    ### Install correct version of automake
    #
    if [ ! -d /usr/local/Cellar/automake/1.12 ]; then
      echo -e "${CLR_NO} Downloading ${CLR_BLUE}automake${CLR_NO}"
      download "automake" "http://ftp.gnu.org/gnu/automake/" "automake-1.12.tar.gz" 2>${TEMPDIR}/error.log || dropout
      echo -e "extracting"
      unzip "automake-1.12.tar.gz" >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -en "${CLR_NO} Downloading ${CLR_BLUE}automake${CLR_NO} ... "
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      mkdir -p build-automake
      pushd build-automake >/dev/null
        rm -rf *

        ### ./configure
        #
        echo -en "${CLR_NO} Configuring ${CLR_BLUE}automake${CLR_NO} ... "
        ../automake-1.12/configure --prefix=/usr/local/Cellar/automake/1.12 >/dev/null 2>${TEMPDIR}/error.log || dropout
        echo -e "${CLR_GREEN}[Done]${CLR_NO}"

        ### make
        #
        echo -en "${CLR_NO} Building ${CLR_BLUE}automake${CLR_NO} ... "
        make all -j >/dev/null 2>${TEMPDIR}/error.log || dropout
        echo -e "${CLR_GREEN}[Done]${CLR_NO}"

        ### make install
        #
        echo -en "${CLR_NO} Installing ${CLR_BLUE}automake${CLR_NO} ... "
        make install >/dev/null 2>${TEMPDIR}/error.log || dropout
        rm -r $PREFIX/share/locale
        echo -e "${CLR_GREEN}[Done]${CLR_NO}"
      popd
    fi

  popd

  brew switch automake 1.12 >/dev/null
}

function prepare_autoconf() {
  pushd $TEMPDIR >/dev/null

    ### Install correct version of autoconf
    #
    if [ ! -d /usr/local/Cellar/autoconf/2.64 ]; then
      echo -e "${CLR_NO} Downloading ${CLR_BLUE}autoconf${CLR_NO}"
      download "autoconf" "http://ftp.gnu.org/gnu/autoconf/" "autoconf-2.64.tar.gz" 2>${TEMPDIR}/error.log || dropout
      echo -e "extracting"
      unzip "autoconf-1.12.tar.gz" >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -en "${CLR_NO} Downloading ${CLR_BLUE}autoconf${CLR_NO} ... "
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      mkdir -p build-autoconf
      pushd build-autoconf >/dev/null
        rm -rf *

        ### ./configure
        #
        echo -en "${CLR_NO} Configuring ${CLR_BLUE}autoconf${CLR_NO} ... "
        ../autoconf-2.64/configure --prefix=/usr/local/Cellar/autoconf/2.64 >/dev/null 2>${TEMPDIR}/error.log || dropout
        echo -e "${CLR_GREEN}[Done]${CLR_NO}"

        ### make
        #
        echo -en "${CLR_NO} Building ${CLR_BLUE}autoconf${CLR_NO} ... "
        make all -j >/dev/null 2>${TEMPDIR}/error.log || dropout
        echo -e "${CLR_GREEN}[Done]${CLR_NO}"

        ### make install
        #
        echo -en "${CLR_NO} Installing ${CLR_BLUE}autoconf${CLR_NO} ... "
        make install >/dev/null 2>${TEMPDIR}/error.log || dropout
        rm -r $PREFIX/share/locale
        echo -e "${CLR_GREEN}[Done]${CLR_NO}"
      popd
    fi
  popd

  brew switch autoconf 2.64 >/dev/null
}

function rebuild_newlib {
  pushd $TEMPDIR >/dev/null

    ### Download and extract newlib
    #
    echo -e "${CLR_NO} Downloading ${CLR_BLUE}${newlibv}${CLR_NO}"
    if [ ! -d $newlibv ]; then
      download "Newlib" "ftp://sourceware.org/pub/newlib/" "${newlibv}.tar.gz" 2>${TEMPDIR}/error.log || dropout
      echo -e "extracting"
      unzip "${newlibv}.tar.gz" >/dev/null 2>${TEMPDIR}/error.log || dropout
    fi
    echo -en "${CLR_NO} Downloading ${CLR_BLUE}${newlibv}${CLR_NO} ... "
    echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    ### Patch newlib
    #
    echo -en "${CLR_NO} Patching ${CLR_BLUE}newlib${CLR_NO} ... "
    if [ ! -f $newlibv/.patched ] || [ "$1" = force ]; then
      dopatch "Newlib" $newlibv $BUILDROOT/toolchain/newlib.patch >/dev/null 2>${TEMPDIR}/error.log || dropout
      pushd $newlibv/newlib/libc/sys >/dev/null
        cp -r $BUILDROOT/toolchain/myos .
        autoconf >/dev/null 2>${TEMPDIR}/error.log || dropout
        pushd myos >/dev/null
          autoreconf >/dev/null 2>${TEMPDIR}/error.log || dropout
        popd >/dev/null
      popd >/dev/null
      touch $newlibv/.patched
    fi
    echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    ### Create or empty build directory
    #
    mkdir -p build-newlib
    pushd build-newlib >/dev/null
      rm -rf *

      ### ./configure
      #
      echo -en "${CLR_NO} Configuring ${CLR_BLUE}newlib${CLR_NO} ... "
      ../${newlibv}/configure --target=$TARGET --prefix=$PREFIX >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make
      #
      echo -en "${CLR_NO} Building ${CLR_BLUE}kernel library${CLR_NO} ... "
      CPPFLAGS_FOR_TARGET=-DKERNEL_MODE make -j >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make install
      #
      echo -en "${CLR_NO} Installing ${CLR_BLUE}kernel library${CLR_NO} ... "
      make install >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"
      mv $PREFIX/$TARGET/lib/libc.a $PREFIX/$TARGET/lib/libkernel.a

      rm -rf *

      ### ./configure
      #
      echo -en "${CLR_NO} Configuring ${CLR_BLUE}newlib${CLR_NO} again ... "
      ../${newlibv}/configure --target=$TARGET --prefix=$PREFIX >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make
      #
      echo -en "${CLR_NO} Building ${CLR_BLUE}user library${CLR_NO} ... "
      make -j >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

      ### make install
      #
      echo -en "${CLR_NO} Installing ${CLR_BLUE}user library${CLR_NO} ... "
      make install >/dev/null 2>${TEMPDIR}/error.log || dropout
      echo -e "${CLR_GREEN}[Done]${CLR_NO}"

    popd >/dev/null
  popd >/dev/null
}

echo -en "${CLR_NO} Checking for ${CLR_BLUE}automake 1.12${CLR_NO} ... "
automake --version | grep 1.12 >/dev/null
if [ $? != 0 ]; then
  echo -e "${CLR_YELLOW}[Missing]${CLR_NO}"
  echo -e "${CLR_BLUE} Building automake 1.12${CLR_NO}"
  prepare_automake
else
  echo -e "${CLR_GREEN}[OK]${CLR_NO}"
fi

echo -en "${CLR_NO} Checking for ${CLR_BLUE}autoconf 2.64${CLR_NO} ... "
autoconf --version | grep 2.64 >/dev/null
if [ $? != 0 ]; then
  echo -e "${CLR_YELLOW}[Missing]${CLR_NO}"
  echo -e "${CLR_BLUE} Building autoconf 2.64${CLR_NO}"
  prepare_autoconf
else
  echo -e "${CLR_GREEN}[OK]${CLR_NO}"
fi

echo -en "${CLR_NO} Checking for ${CLR_BLUE}newlib${CLR_NO} ... "
if [ ! -f $PREFIX/$TARGET/lib/libc.a ]; then
  echo -e "${CLR_RED}[Missing]${CLR_NO}"
  echo -e "${CLR_BLUE} Building newlib${CLR_NO}"
  rebuild_newlib
else
  if [ "$1" = "force" ]; then
    echo -e "${CLR_YELLOW}[Forced]${CLR_NO}"
    rebuild_newlib force
  else
    echo -e "${CLR_GREEN}[OK]${CLR_NO}"
  fi
fi
