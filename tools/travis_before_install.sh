#!/bin/bash

if [[ $(uname -s) = "Darwin" ]]; then
  echo "Darwin before install start"
  echo "Darwin before install end"
fi

if [[ $(uname -s) = "Linux" ]]; then
  echo "Linux before install start"
  # We need this to have g++4.8 available in apt
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt-get update -qq
  echo "Linux before install end"
fi

