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

build_config "libscalloc" ""
build_config "libscalloc-reuse-0" "-D lab_model=SCALLOC_LAB_MODEL_TLAB -Dreuse_threshold=0 "
build_config "libscalloc-reuse-20" "-D lab_model=SCALLOC_LAB_MODEL_TLAB -Dreuse_threshold=20"
build_config "libscalloc-reuse-40" "-D lab_model=SCALLOC_LAB_MODEL_TLAB -Dreuse_threshold=40"
build_config "libscalloc-reuse-60" "-D lab_model=SCALLOC_LAB_MODEL_TLAB -Dreuse_threshold=60"
build_config "libscalloc-reuse-80" "-D lab_model=SCALLOC_LAB_MODEL_TLAB -Dreuse_threshold=80"
build_config "libscalloc-reuse-100" "-D lab_model=SCALLOC_LAB_MODEL_TLAB -Dreuse_threshold=100"

cd ..

