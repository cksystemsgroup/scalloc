#!/bin/bash

# Custom script for ACDC to build many different flavors of scalloc.
#
# Usage:
# * Check out and prepare ACDC as described in
#     github.com/cksystemsgroup/acdc
# * Download install_scalloc.sh into the top-level ACDC directory.
#     https://github.com/cksystemsgroup/scalloc/blob/master/tools/install_scalloc.sh
# * Run ./install_allocators scalloc

rm -rf scalloc
git clone git@github.com:cksystemsgroup/scalloc.git
cd scalloc/
tools/make_deps.sh

build/gyp/gyp --depth=. -Deager_madvise_threshold=65536 scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-eager.so

build/gyp/gyp --depth=. -D core_local=1 scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-core-local.so

build/gyp/gyp --depth=. -D max_backends=1 -D dq_backend=LockedStack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-1-locked.so

build/gyp/gyp --depth=. -D max_backends=5 -D dq_backend=LockedStack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-5-locked.so

build/gyp/gyp --depth=. -D max_backends=10 -D dq_backend=LockedStack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-10-locked.so

build/gyp/gyp --depth=. -D max_backends=20 -D dq_backend=LockedStack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-20-locked.so

build/gyp/gyp --depth=. -D max_backends=40 -D dq_backend=LockedStack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-40-locked.so

build/gyp/gyp --depth=. -D max_backends=80 -D dq_backend=LockedStack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-80-locked.so

build/gyp/gyp --depth=. -D max_backends=1 -D dq_backend=Stack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-1-lockfree.so

build/gyp/gyp --depth=. -D max_backends=5 -D dq_backend=Stack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-5-lockfree.so

build/gyp/gyp --depth=. -D max_backends=10 -D dq_backend=Stack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-10-lockfree.so

build/gyp/gyp --depth=. -D max_backends=20 -D dq_backend=Stack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-20-lockfree.so

build/gyp/gyp --depth=. -D max_backends=40 -D dq_backend=Stack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-40-lockfree.so

build/gyp/gyp --depth=. -D max_backends=80 -D dq_backend=Stack scalloc.gyp
BUILDTYPE=Release V=1 make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc-80-lockfree.so

./build/gyp/gyp --depth=. scalloc.gyp
BUILDTYPE=Release make
cp out/Release/lib.target/libscalloc.so out/Release/libscalloc.so

cd ..
rm -rf libscalloc*
ln -s scalloc/out/Release/libscalloc.so libscalloc.so
ln -s scalloc/out/Release/libscalloc.so libscalloc.so.0
ln -s scalloc/out/Release/libscalloc-eager.so libscalloc-eager.so
ln -s scalloc/out/Release/libscalloc-eager.so libscalloc-eager.so.0
ln -s scalloc/out/Release/libscalloc-core-local.so libscalloc-core-local.so
ln -s scalloc/out/Release/libscalloc-core-local.so libscalloc-core-local.so.0
ln -s scalloc/out/Release/libscalloc-1-locked.so libscalloc-1-locked.so
ln -s scalloc/out/Release/libscalloc-1-locked.so libscalloc-1-locked.so.0
ln -s scalloc/out/Release/libscalloc-5-locked.so libscalloc-5-locked.so
ln -s scalloc/out/Release/libscalloc-5-locked.so libscalloc-5-locked.so.0
ln -s scalloc/out/Release/libscalloc-10-locked.so libscalloc-10-locked.so
ln -s scalloc/out/Release/libscalloc-10-locked.so libscalloc-10-locked.so.0
ln -s scalloc/out/Release/libscalloc-20-locked.so libscalloc-20-locked.so
ln -s scalloc/out/Release/libscalloc-20-locked.so libscalloc-20-locked.so.0
ln -s scalloc/out/Release/libscalloc-40-locked.so libscalloc-40-locked.so
ln -s scalloc/out/Release/libscalloc-40-locked.so libscalloc-40-locked.so.0
ln -s scalloc/out/Release/libscalloc-80-locked.so libscalloc-80-locked.so
ln -s scalloc/out/Release/libscalloc-80-locked.so libscalloc-80-locked.so.0
ln -s scalloc/out/Release/libscalloc-1-lockfree.so libscalloc-1-lockfree.so
ln -s scalloc/out/Release/libscalloc-1-lockfree.so libscalloc-1-lockfree.so.0
ln -s scalloc/out/Release/libscalloc-5-lockfree.so libscalloc-5-lockfree.so
ln -s scalloc/out/Release/libscalloc-5-lockfree.so libscalloc-5-lockfree.so.0
ln -s scalloc/out/Release/libscalloc-10-lockfree.so libscalloc-10-lockfree.so
ln -s scalloc/out/Release/libscalloc-10-lockfree.so libscalloc-10-lockfree.so.0
ln -s scalloc/out/Release/libscalloc-20-lockfree.so libscalloc-20-lockfree.so
ln -s scalloc/out/Release/libscalloc-20-lockfree.so libscalloc-20-lockfree.so.0
ln -s scalloc/out/Release/libscalloc-40-lockfree.so libscalloc-40-lockfree.so
ln -s scalloc/out/Release/libscalloc-40-lockfree.so libscalloc-40-lockfree.so.0
ln -s scalloc/out/Release/libscalloc-80-lockfree.so libscalloc-80-lockfree.so
ln -s scalloc/out/Release/libscalloc-80-lockfree.so libscalloc-80-lockfree.so.0
