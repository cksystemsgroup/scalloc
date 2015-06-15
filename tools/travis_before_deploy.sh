#!/bin/bash

if [[ $(uname -s) = "Linux" ]]; then
  echo "Linux before deploy start"
  cp out/Release/lib.target/libscalloc.so libscalloc.so
  cp out/Debug/lib.target/libscalloc.so libscalloc-dbg.so
  tar -czf binaries-$(uname -s)-$(uname -i).tar.gz libscalloc*
  echo "Linux before deploy end"
fi

