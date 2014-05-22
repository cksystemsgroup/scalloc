#!/bin/bash

./tools/cpplint.py --filter=-build/header_guard,-build/include \
  src/* \
  src/allocators/* \
  src/buffer/*

exit $?
