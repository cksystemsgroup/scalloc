#!/bin/bash

echo "updating dependencies for scalloc"
echo ""

echo -n "checking out gyp... "
svn checkout --quiet --force http://gyp.googlecode.com/svn/trunk build/gyp --revision 1859
if [ $? -eq 0 ]; then
  echo  "done"
else
  exit $?
fi
echo -n "checking out gtest... "
svn checkout --quiet --force http://googletest.googlecode.com/svn/trunk third_party/gtest --revision 680 
if [ $? -eq 0 ]; then
  echo  "done"
else
  exit $?
fi
