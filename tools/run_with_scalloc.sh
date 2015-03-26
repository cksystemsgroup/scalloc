#!/bin/bash

TYPE="Debug"
LIB_BASE="./out/$TYPE"

if [[ $(uname -s) = "Darwin" ]]; then
  DYLD_INSERT_LIBRARIES=${LIB_BASE}/libscalloc.dylib \
  DYLD_FORCE_FLAT_NAMESPACE=1 \
  $*
fi

if [[ $(uname -s) = "Linux" ]]; then
  LD_PRELOAD=${LIB_BASE}/libscalloc.so \
  $*
fi


