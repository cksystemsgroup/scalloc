#!/bin/bash

mkdir thirdy_party

svn checkout --force http://gyp.googlecode.com/svn/trunk build/gyp --revision 1859
svn checkout --force http://googletest.googlecode.com/svn/trunk third_party/gtest --revision 680 
