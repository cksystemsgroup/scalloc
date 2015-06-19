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
  git clone https://github.com/cksystemsgroup/scalloc.git scalloc
  cd scalloc/
  rm -rf out
fi

tools/make_deps.sh

BUILD_OPTS="-j 24"

export V=1

function build_config() {
  name=$1
  params=$2

  build/gyp/gyp --depth=. $params scalloc.gyp
  BUILDTYPE=Release make $BUILD_OPTS
  BUILDTYPE=Debug make $BUILD_OPTS
  cp out/Release/lib.target/libscalloc.so out/Release/${name}.so
  cp out/Debug/lib.target/libscalloc.so out/Debug/${name}.so
  ln -s scalloc/out/Release/${name}.so ../${name}.so
  ln -s scalloc/out/Release/${name}.so ../${name}.so.0
}

build_config "libscalloc" \
             "-Dsafe_global_construction=yes"
build_config "libscalloc-no-madvise" \
             "-Dmadvise=no -Dsafe_global_construction=yes"
build_config "libscalloc-sp-1-backend" \
             "-Dspan_pool_backend_limit=1 -Dsafe_global_construction=yes"
build_config "libscalloc-no-cleanup-in-free" \
             "-Dcleanup_in_free=no -Dsafe_global_construction=yes"

build_config "libscalloc-no-reuse" \
             "-Dreuse_threshold=100 -Dsafe_global_construction=yes"

cd ..

