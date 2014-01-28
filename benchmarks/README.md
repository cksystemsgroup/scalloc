# Benchmarking scalloc

## AC/DC

## Hoard

## Other

### thread-termination

A benchmark checking whether thread-local buffers are properly reused or emptied. The benchmark creates a single producer (allocating memory) and a single consumer (freeing memory) in a loop. The goal is to show that allocators reach a steady state (have no leak) wrt. to the resident set size (RSS).
