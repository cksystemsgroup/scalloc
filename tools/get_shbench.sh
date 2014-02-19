#!/bin/bash

curl -o benchmarks/shbench/bench.zip http://www.microquill.com/smartheap/shbench/bench.zip
cd benchmarks/shbench/
unzip bench.zip
rm bench.zip
