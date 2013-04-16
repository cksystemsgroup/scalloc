# We'll add the programs throughout the Makefile.
bin_PROGRAMS =
noinst_PROGRAMS =
TESTS = 

AM_CPPFLAGS = \
	-Wall -Werror \
	-Winline \
	-ggdb \
	-mcx16 \
	-m64  \
	-std=c++11 \
        -I${top_srcdir}/src \
        -ffast-math \
        -fno-builtin-malloc \
        -fno-builtin-calloc \
        -fno-builtin-realloc \
        -fno-builtin-free \
        -fno-omit-frame-pointer

AM_LDFLAGS = -Wl,--as-needed 
LDADD = 

lib_LTLIBRARIES = libscalloc.la
# also include .h files into sources, since the distribution tarball is
# generated from this list
libscalloc_la_SOURCES = \
	src/freelist.h \
	src/log.cc \
	src/log.h
libscalloc_la_LDFLAGS = -version-info 0:0:0

BENCH_CPPFLAGS = \
	-m64 \
	-O3

TEST_CPPFLAGS = \
	-Wall -Werror \
	-std=c++11 \
	-I$(top_srcdir)/src \
	-m64

#
# Unit tests using googletest
#

gtest:
	$(AM_V_GEN)mkdir -p gtest
	cd gtest ; cmake /usr/src/gtest
	$(AM_V_GEN)make -C gtest all

gtest/libgtest_main.a: gtest

gtest/libgtest.a: gtest

gtest-clean:
	$(AM_V_GEN)rm -rf gtest

GTEST_LIBS = gtest/libgtest.a gtest/libgtest_main.a

TESTS += freelist_unittest
freelist_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
freelist_unittest_LDADD = \
	$(GTEST_LIBS)
freelist_unittest_SOURCES = \
	src/log.cc \
	src/test/freelist_unittest.cc

noinst_PROGRAMS += $(TESTS)

#
# Extend the standard Makefile rules:
#
