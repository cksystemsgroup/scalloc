#!/bin/bash

if [[ $(uname -s) = "Darwin" ]]; then
fi

if [[ $(uname -s) = "Linux" ]]; then
  sudo apt-get install -qq gcc-4.8 g++-4.8 
  # We want to compile with g++ 4.8 when rather than the default g++
  sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90
fi

