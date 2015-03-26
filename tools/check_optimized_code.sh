#!/bin/bash

WHITELIST=(
  "_Z9LogPrintfiPKcS0_z"      # printf used in logging
  "_ZN7scalloc12ScallocGuard" # guard
  "_ZN12_GLOBAL__N_111exitHa" # exit handler (func ptr)
  "_Z10LogPrintf2iPKcS0_z"    # printf with var args
  "_ZNSt13__atomic_baseImE23" # atomic base implementation (constructor)
  "_ZN7scalloc27ThreadLocalA" # TLS dtor
)


echo "Potential performance bottlenecks:"

readelf -s out/Release/lib.target/libscalloc.so  \
    | grep FUNC \
    | grep _Z \
    | awk '{print $8}' \
    | while read token; do
  found=0
  for i in "${WHITELIST[@]}"; do
    if [[ "$i" = "$token" ]]; then
      found=1
    fi
  done
  if [[ $found -eq 0 ]]; then
    echo "  $token"
  fi
done


