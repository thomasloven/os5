#!/usr/bin/env bash

C_NO=`echo -e "\\033[0m"`
C_RED=`echo -e "\\033[31m"`
C_GREEN=`echo -e "\\033[32m"`
C_YELLOW=`echo -e "\\033[33m"`
C_BLUE=`echo -e "\\033[36m"`

while read line; do
  echo $line | sed \
    -e "s/\(\[info\]\)/${C_BLUE}\1${C_NO}/" \
    -e "s/\(\[status\]\)/${C_GREEN}\1${C_NO}/" \
    -e "s/\(\[warning\]\)/${C_YELLOW}\1${C_NO}/" \
    -e "s/\(\[error\]\)/${C_RED}\1${C_NO}/"
done

