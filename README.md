# scalloc

Fast and scalable memory allocation

## Dependencies

You need a recent compiler capable of compiling C++11 (c++0x) code.  Furthermore
it is assumed that  `__thread` is present (until we move to C++11's
`thread_local`).

* autotools (automake and friends)
* pkg-config

**Building with unittests (--enable-tests):**
* cmake (for googletest)
* [googletest](https://code.google.com/p/googletest "googletest")

## Building [![Build Status](https://drone.io/github.com/cksystemsgroup/scalloc/status.png)](https://drone.io/github.com/cksystemsgroup/scalloc/latest)

Checkout the latest release version

    git clone https://github.com/cksystemsgroup/scalloc.git
    cd scalloc
    git checkout release

Then buildig is as easy as

    ./autogen.sh
    ./configure
    make

See `./configure --help` for optional features, for instance compiling with
additional integrity checks.

Various tests can be performed with

    make check
    
If you just want a prebuilt binary, check out the [download section at drone.io](https://drone.io/github.com/cksystemsgroup/scalloc/files "scalloc downloads").
 
## Usage

In order to make use of scalloc, just preload it using `LD_PRELOAD`.

    LD_PRELOAD=/path/to/libscalloc.so ./foo

## License

Copyright (c) 2012-2013, the scalloc Project Authors.
All rights reserved. Please see the AUTHORS file for details.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the scalloc Project.
