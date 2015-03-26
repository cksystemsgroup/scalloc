#!/bin/bash

make -j 2
BUILDTYPE=Release make

echo ""
./contrib/check_optimized_code.sh

