#!/bin/sh

# strict mode
set -eu

# compile dwm
ret=""
if make clean && make && strip dwm; then
  echo "OK"
  ret=0
else
  echo "NOK"
  ret=1
fi
exit $ret
