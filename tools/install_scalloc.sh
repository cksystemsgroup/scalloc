#!/bin/bash

# Setup:
# * Check out and prepare ACDC as described in
#     github.com/cksystemsgroup/acdc
# * Check out and prepare scalloc as described in 
#     github.com/cksystemsgroup/scalloc
# * Assuming assuming scalloc/ and acdc/ sub directories:
#     cd acdc/
#     ln -s ../scalloc/tools/install_scalloc.sh install_scalloc.sh
#     ./install_allocators scalloc

rm -rf libscalloc*

if [ -d scalloc ]; then
  cd scalloc
  git pull
  rm -rf out
else
  git clone https://github.com/cksystemsgroup/scalloc.git
  cd scalloc/
  rm -rf out
fi

tools/make_deps.sh

BUILD_OPTS="-j 80"

export BUILDTYPE=Release
export V=1

function build_config() {
  name=$1
  params=$2


  build/gyp/gyp --depth=. $params scalloc.gyp
  make $BUILD_OPTS
  cp out/Release/lib.target/libscalloc.so out/Release/${name}.so
  ln -s scalloc/out/Release/${name}.so ../${name}.so
  ln -s scalloc/out/Release/${name}.so ../${name}.so.0
}

build_config "libscalloc-clab" "-D core_local=1 -D incremental_freelist=1 -D size_class_config=optimized"
build_config "libscalloc-tlab" "-D core_local=-1 -D incremental_freelist=1 -D size_class_config=optimized"
build_config "libscalloc-eager" "-D core_local=1 -D eager_madvise_threshold=4096 -D incremental_freelist=1 -D size_class_config=optimized"
build_config "libscalloc-hugepage" "-D core_local=1 -D clab_policy=rr -D size_class_config=hugepage -D incremental_freelist=1" 

#build_config "libscalloc-eager" "-D core_local=1 -D clab_policy=rr -D eager_madvise_threshold=4096 -D incremental_freelist=1"
#build_config "libscalloc-core-local" "-D core_local=1 -D clab_policy=utilization"
#build_config "libscalloc-core-local-1mb" "-D core_local=1 -D clab_policy=rr  -D size_classes_1mb=1 -D eager_madvise_threshold=4096"
#build_config "libscalloc-active-threads" "-D core_local=1 -D clab_policy=threads"
#build_config "libscalloc-static" "-D core_local=1 -D clab_policy=rr"
#build_config "libscalloc-hugepage" "-D core_local=1 -D clab_policy=rr -D huge_pages=1 -D incremental_freelist=1" 
#build_config "libscalloc-eager-reuse" "-D core_local=1 -D clab_policy=rr -D enable_slow_span_reuse=1"
#build_config "libscalloc-lazy-init" "-D core_local=1 -D clab_policy=rr -D enable_free_list_reuse=1"
#build_config "libscalloc-tlab" "-D core_local=-1 -D clab_policy=rr"
for i in 1 2 4 8 16 32 64
do
  echo "test: $i"
  build_config "libscalloc-$i-locked" "-D core_local=1 -D max_parallelism=$i -D dq_backend=LockedStack"
  build_config "libscalloc-$i-lockfree" "-D core_local=1 -D max_parallelism=$i -D dq_backend=Stack"
  #build_config "libscalloc-$i-locked-profile" "-D core_local=1 -D clab_policy=rr -D max_parallelism=$i -D dq_backend=LockedStack -D profile=1"
  #build_config "libscalloc-$i-lockfree-profile" "-D core_local=1 -D clab_policy=rr -D max_parallelism=$i -D dq_backend=Stack -D profile=1"
done

cd ..

