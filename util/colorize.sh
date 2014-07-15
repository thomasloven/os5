#!/usr/bin/env bash

. ./util/common.sh

while read line; do
  echo "$line" | sed \
    -e "s/\(\[info\]\)/$(ce ${C_BLUE})\1$(ce ${C_NO})/" \
    -e "s/\(\[status\]\)/$(ce ${C_GREEN})\1$(ce ${C_NO})/" \
    -e "s/\(\[warning\]\)/$(ce ${C_YELLOW})\1$(ce ${C_NO})/" \
    -e "s/\(\[error\]\)/$(ce ${C_RED})\1$(ce ${C_NO})/"
done

