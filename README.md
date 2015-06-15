# scalloc [![Build Status](https://travis-ci.org/cksystemsgroup/scalloc.svg?branch=master)](https://travis-ci.org/cksystemsgroup/scalloc)

**Note** 

We completely rewrote the allocator from scratch. Please use the current version to
obtain benchmarking numbers for scalloc. See [Benchmarking](#benchmarking).

**A Fast, Multicore-Scalable, Low-Fragmentation Memory Allocator**

The problem of concurrent memory allocation is to find the right balance between
temporal and spatial performance and scalability across a large range of
workloads. Our contributions to address this problem are: uniform treatment of
small and big objects through the idea of virtual spans, efficiently and
effectively reclaiming unused memory through fast and scalable global data
structures, and constant-time (modulo synchronization) allocation and
deallocation operations that trade off memory reuse and spatial locality without
being subject to false sharing. We have implemented an allocator, scalloc, based
on these ideas that generally performs and scales in our experiments better than
other allocators while using less memory and is still competitive otherwise.

**Contents**
* [Binaries](#binaries)
* [Building](#building-from-source)
* [Using scalloc](#using-scalloc)
* [Benchmarking](#benchmarking)

**Disclaimer**

While the code provided in this repository is complete in the sence that it supports
the full API, it may still require tuning [for a specific production environment  (read: *your* 
environment)](http://en.wikipedia.org/wiki/Pareto_principle).

## Binaries

We do not explicitly provide pre-built binaries. However, binaries for Linux
(x86_64) can be found in the [release section](https://github.com/cksystemsgroup/scalloc/releases)
of this repository.

## Building from source

We support building on OS X (>10.6) and Linux-based systems using gyp. The only
requirement to actually get started is a working `git` installation.

### Setting up a build environment

Checkout the latest version
```sh
git clone https://github.com/cksystemsgroup/scalloc.git
cd scalloc
```

If you don't have a global gyp installation, you can get a local one using
```sh
tools/make_deps.sh
```

Generate a build environment (using the gyp installation from the previous step)
```sh
build/gyp/gyp --depth=. scalloc.gyp
```

Additionally, scalloc provides some compile-time configuration flags:
* log_level: Log level that is used through the allocator. [default: kWarning]
* reuse_threshold: Utilization of spans that should be revived before they
  actually get empty (i.e. all objects have been returned). A threshold of 100
  corresponds to disabling this feature at compile time. [default: 80]

Flags may be set when creating the build files using `gyp` by passing them as flags, i.e.,
`-Dflag=value`. For example, `-Dreuse_threshold=20`.

We support the following build configurations:

* **Debug**: Binaries are created with debugging symbols and without optimizations. 
  We also include assertions checking for various invariants.
* **Release**: Binaries are created with maximum optimization levels, no debugging 
  symbols, and without assertions.

### ... on Linux

After setting up the build environment building scalloc is as easy as
```sh
BUILDTYPE=Debug make # default
BUILDTYPE=Release make
```

### ... on OSX

Open `scalloc.xcodeproj` and build the project using Xcode, or build it from the command
line using
```sh
build/gyp/gyp --depth=. scalloc.gyp --build=Release
```

## Using scalloc

### ... on Linux

In order to make use of scalloc, preload it using `LD_PRELOAD`.
```sh
LD_PRELOAD=/path/to/libscalloc.so ./foo
```

scalloc heavily makes use of 64bit address space. If you run into mmap limits
you  need to disable overcommit accounting. Additionally, make sure that
transparent huge pages are disabled. On recent versions of Linux
you can do this by
```sh
sudo sh -c "echo 1 > /proc/sys/vm/overcommit_memory"
sudo sh -c "echo never > /sys/kernel/mm/transparent_hugepage/enabled"
```

See the kernel docs on [overcommit accounting](https://www.kernel.org/doc/Documentation/vm/overcommit-accounting)
and [transparent hugepages](https://www.kernel.org/doc/Documentation/vm/transhuge.txt) for more information.

### ... on OSX

Similar to preloading on Linux, one can preload scalloc using
```sh
DYLD_INSERT_LIBRARIES=/path/to/libscalloc.dylib DYLD_FORCE_FLAT_NAMESPACE=1 ./foo
```

## Benchmarking

See [cksystemsgroup/scalloc-artifact](https://github.com/cksystemsgroup/scalloc-artifact) for
setting up a benchmarking environment to compare scalloc against other allocators.

