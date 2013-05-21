#!/bin/bash

./tools/cpplint.py --filter=-build/header_guard,-build/include $@

exit $?
