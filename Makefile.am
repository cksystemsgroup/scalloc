# We'll add the programs throughout the Makefile.
bin_PROGRAMS =
noinst_PROGRAMS =
TESTS = 

AM_CPPFLAGS = \
	-Wall -Werror \
	-mcx16 \
	-m64  \
	-std=c++0x \
        -I${top_srcdir}/src \
        -ffast-math \
        -fno-omit-frame-pointer

if DEBUG
AM_CPPFLAGS += -ggdb
else
AM_CPPFLAGS += -O3
endif

AM_LDFLAGS = -Wl,--as-needed 
LDADD = 

lib_LTLIBRARIES = libscalloc.la
# also include .h files into sources, since the distribution tarball is
# generated from this list
libscalloc_la_SOURCES = \
	src/allocators/dq_sc_allocator.cc \
	src/allocators/dq_sc_allocator.h \
	src/allocators/global_sbrk_allocator.cc \
	src/allocators/global_sbrk_allocator.h \
	src/allocators/large_object_allocator.h \
	src/allocators/page_heap.cc \
	src/allocators/page_heap.h \
	src/allocators/slab_sc_allocator.cc \
	src/allocators/slab_sc_allocator.h \
	src/atomic.h \
	src/block_header.h \
	src/common.h \
	src/distributed_queue.cc \
	src/distributed_queue.h \
	src/freelist.h \
	src/log.cc \
	src/log.h \
	src/random.h \
	src/runtime_vars.cc \
	src/runtime_vars.h \
	src/scalloc.cc \
	src/scalloc.h \
	src/scalloc_guard.h \
	src/size_map.cc \
	src/size_map.h \
	src/thread_cache.cc \
	src/thread_cache.h \
	src/page_heap_allocator.h \
	src/profiler.cc \
	src/profiler.h \
	src/spinlock-inl.h \
	src/stack-inl.h \
	src/system-alloc.cc \
	src/system-alloc.h
libscalloc_la_LDFLAGS = -version-info 0:0:0

BENCH_CPPFLAGS = \
	-m64 \
	-O3

TEST_CPPFLAGS = \
	-Wall -Werror \
	-std=c++0x \
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

if UNITTESTS

TESTS += atomic_unittest
atomic_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
atomic_unittest_LDADD = \
	$(GTEST_LIBS)
atomic_unittest_SOURCES = \
	src/test/atomic_unittest.cc

TESTS += distributed_queue_unittest
distributed_queue_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
distributed_queue_unittest_LDADD = \
	$(GTEST_LIBS)
distributed_queue_unittest_SOURCES = \
	src/allocators/global_sbrk_allocator.cc \
	src/distributed_queue.cc \
	src/distributed_queue.h \
	src/log.cc \
	src/log.h \
	src/page_heap_allocator.h \
	src/runtime_vars.cc \
	src/system-alloc.cc \
	src/test/distributed_queue_unittest.cc

TESTS += freelist_unittest
freelist_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
freelist_unittest_LDADD = \
	$(GTEST_LIBS)
freelist_unittest_SOURCES = \
	src/log.cc \
	src/test/freelist_unittest.cc

TESTS += large_object_allocator_unittest
large_object_allocator_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
large_object_allocator_unittest_LDADD = \
	$(GTEST_LIBS)
large_object_allocator_unittest_SOURCES = \
	src/log.cc \
	src/runtime_vars.cc \
	src/system-alloc.cc \
	src/test/large_object_allocator_unittest.cc

TESTS += log2_unittest
log2_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
log2_unittest_LDADD = \
	$(GTEST_LIBS)
log2_unittest_SOURCES = \
	src/test/log2_unittest.cc

TESTS += page_heap_allocator_unittest
page_heap_allocator_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
page_heap_allocator_unittest_LDADD = \
	$(GTEST_LIBS)
page_heap_allocator_unittest_SOURCES = \
	src/allocators/global_sbrk_allocator.cc \
	src/log.cc \
	src/test/page_heap_allocator_unittest.cc \
	src/runtime_vars.cc \
	src/system-alloc.cc

TESTS += page_heap_unittest
page_heap_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
page_heap_unittest_LDADD = \
	$(GTEST_LIBS)
page_heap_unittest_SOURCES = \
	src/allocators/global_sbrk_allocator.cc \
	src/allocators/page_heap.cc \
	src/distributed_queue.cc \
	src/log.cc \
	src/test/page_heap_unittest.cc \
	src/runtime_vars.cc \
	src/system-alloc.cc

TESTS += runtime_vars_unittest
runtime_vars_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
runtime_vars_unittest_LDADD = \
	$(GTEST_LIBS)
runtime_vars_unittest_SOURCES = \
	src/log.cc \
	src/runtime_vars.cc \
	src/test/runtime_vars_unittest.cc

TESTS += slab_sc_allocator_unittest
slab_sc_allocator_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
slab_sc_allocator_unittest_LDADD = \
	$(GTEST_LIBS)
slab_sc_allocator_unittest_SOURCES = \
	src/allocators/dq_sc_allocator.cc \
	src/allocators/global_sbrk_allocator.cc \
	src/allocators/page_heap.cc \
	src/allocators/slab_sc_allocator.cc \
	src/distributed_queue.cc \
	src/log.cc \
	src/size_map.cc \
	src/runtime_vars.cc \
	src/system-alloc.cc \
	src/test/slab_sc_allocator_unittest.cc

TESTS += stack_unittest
stack_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
stack_unittest_LDADD = \
	$(GTEST_LIBS)
stack_unittest_SOURCES = \
	src/test/stack_unittest.cc

TESTS += system_alloc_unittest
system_alloc_unittest_CPPFLAGS = \
	$(TEST_CPPFLAGS)
system_alloc_unittest_LDADD = \
	$(GTEST_LIBS)
system_alloc_unittest_SOURCES = \
	src/log.cc \
	src/runtime_vars.cc \
	src/system-alloc.cc \
	src/test/system_alloc_unittest.cc

endif

noinst_PROGRAMS += $(TESTS)

#
# Extend the standard Makefile rules:
#
