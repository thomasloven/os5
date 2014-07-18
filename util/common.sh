#!/usr/bin/env bash

readonly STASH=~/osdev/stash
readonly TOOLCHAIN=~/osdev/toolchain
readonly PREFIX=${TOOLCHAIN}
readonly TARGET=i586-pc-myos

readonly C_NO="\\033[0m"
readonly C_RED="\\033[31m"
readonly C_GREEN="\\033[32m"
readonly C_YELLOW="\\033[33m"
readonly C_BLUE="\\033[36m"
function ce()
{
  echo -e "${@}"
}

function fail() {
  echo -e "${C_RED}[FAILED]${C_NO}"
  exit 1
}
function print_done() {
  local spaces=$1

  echo -e "${spaces}${C_GREEN}[DONE]${C_NO}"
}
function print_skip() {
  local spaces=$1

  echo -e "${spaces}${C_YELLOW}[SKIP]${C_NO}"
}
function print_task() {
  local task=$1
  local package=$2

  echo -e "${task} ${C_BLUE}${package}${C_NO}"
}

function download() {
  local package=$1
  local url=$2

  local filename=$(basename "${url}")

  pushd "${STASH}" >/dev/null
  print_task "  Downloading" "${package}"
  if [[ ! -f ${filename} ]]; then
    /usr/bin/env curl -# -O "${url}" || fail
    print_done "  "
  else
    print_skip "  "
  fi
  popd >/dev/null
}

function unpack() {
  local package=$1

  pushd "${STASH}" >/dev/null
  print_task "  Unpacking" "${package}"
  if [[ ! -d ${STASH}/${package} ]]; then
    /usr/bin/env tar -xf "${package}.tar.gz"
    print_done "  "
  else
    print_skip "  "
  fi
  popd >/dev/null
}

function patch() {
  local package=$1
  local patch=$2

  local patchname=$(basename "${patch}")

  pushd "${STASH}/${package}" >/dev/null
  print_task "  Patching" "${package}"
  if [[ ! -f .patch-${patchname} ]]; then
    /usr/bin/env patch -p1 -N < "${patch}"
    touch ".patch-${patchname}"
    print_done "  "
  else
    print_skip "  "
  fi
  popd >/dev/null
}


function get_package() {
  local package=$1 ; shift
  local url=$1 ; shift
  local patches="${@}"

  print_task " Preparing" "${package}"
  download "${package}" "${url}"
  unpack "${package}"
  local i
  for i in ${patches}
  do
    patch "${package}" "${i}"
  done
  print_done " "
}

function build_package() {
  local package=$1
  local checkfile=$2
  local configflags=$3
  local makesuffix=$4

  print_task " Building"" ${package}"
  if [[ ! -f ${checkfile} ]]; then
    mkdir -p "${STASH}/build-${package}"
    pushd "${STASH}/build-${package}" >/dev/null || fail
    rm -rf ./*

    print_task "  Configuring"" ${package}"
    ../${package}/configure \
      --prefix=${TOOLCHAIN} \
      ${configflags} \
      >/dev/null 2>"${STASH}/error.log" || fail
    print_done "  "

    print_task "  Making" "${package}"
    make -j all${makesuffix} \
      >/dev/null 2>"${STASH}/error.log" || fail
    print_done "  "

    print_task "  Installing"" ${package}"
    make -j install${makesuffix} \
      >/dev/null 2>"${STASH}/error.log" || fail
    print_done "  "

    print_done " "
  popd >/dev/null
  else
    print_skip " "
  fi
}
