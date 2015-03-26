#!/bin/bash

./tools/cpplint.py --filter=-build/header_guard,-build/include,-readability/todo \
  src/* \
  src/platform/*

exit $?
