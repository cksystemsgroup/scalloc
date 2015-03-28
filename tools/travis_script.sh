#!/bin/bash

if [[ $(uname -s) = "Darwin" ]]; then
  build/gyp/gyp --depth=. scalloc.gyp --build=Debug
  build/gyp/gyp --depth=. scalloc.gyp --build=Release
fi

if [[ $(uname -s) = "Linux" ]]; then
  build/gyp/gyp --depth=. scalloc.gyp
  V=1 BUILDTYPE=Debug make
  V=1 BUILDTYPE=Release make
fi

