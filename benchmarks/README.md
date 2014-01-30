# Benchmarking scalloc

## AC/DC

For now, see https://github.com/cksystemsgroup/acdc.

## Hoard Benchmarks

For now, see https://github.com/emeryberger/Hoard/tree/master/benchmarks/.

Licensing information can be found [here](https://github.com/cksystemsgroup/scalloc/tree/master/benchmarks/hoard).

### threadtest

Parameters: \<threads\> \<rounds\> \<#objects\> \<work\> \<obj-size\>

### larson

Parameters: \<sleep\> \<min-size\> \<max-size\> \<chunks-per-thread\> \<rounds\> \<rng-seed\> \<threads\>

## Other

### thread-termination

A benchmark checking whether thread-local buffers are properly reused or emptied. The benchmark creates a single producer (allocating memory) and a single consumer (freeing memory) in a loop. The goal is to show that allocators reach a steady state (have no leak) wrt. to the resident set size (RSS).
