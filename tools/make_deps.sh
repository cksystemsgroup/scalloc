#!/bin/bash

echo "updating dependencies for scalloc"
echo ""

echo "gyp... "
echo ""

if [[ -d build/gyp ]]; then
  cd build/gyp
  git pull
else
  mkdir -p build/gyp
  git clone https://chromium.googlesource.com/external/gyp build/gyp
fi

if [ $? -eq 0 ]; then
  echo ""
  echo "gyp... done"
else
  exit $?
fi


