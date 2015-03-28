#!/bin/bash

if [[ $(uname -s) = "Darwin" ]]; then
fi

if [[ $(uname -s) = "Linux" ]]; then
  # We need this to have g++4.8 available in apt
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt-get update -qq
fi

