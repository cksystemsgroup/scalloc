AC_INIT([scalloc], 1.0)

AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_HEADER([src/config.h])

: ${CFLAGS=""}
: ${CXXFLAGS=""}

AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])

LT_INIT

dnl uintxx_t types
AC_CHECK_HEADERS([stdint.h inttypes.h sys/types.h],
    [scalloc_found_int_headers=yes; break;])
AS_IF([test "x$scalloc_found_int_headers" != "xyes"],
    [AC_MSG_ERROR([Unable to find the standard integers headers])])

dnl pthreads
AC_CHECK_HEADERS([pthread.h],
    [scalloc_found_pthread=yes; break;])
AS_IF([test "x$scalloc_found_pthread" != "xyes"],
    [AC_MSG_ERROR([Unable to find pthread headers])])

AC_SEARCH_LIBS([pthread_create], [pthread dld], [], [
  AC_MSG_ERROR([unable to find the pthread_create() function])
])

dnl math stuff
AC_SEARCH_LIBS([fabs], [m], [], [
  AC_MSG_ERROR([unable to find fabs() function of the math library])
])

dnl some features

dnl enable unittests
AC_ARG_ENABLE([tests],
    AS_HELP_STRING([--enable-tests], [Enable unittests]))
AM_CONDITIONAL([UNITTESTS], [test x$enable_tests = xyes])

dnl optimization level
AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug], [Enable compilation with debug symbols and
                    disable optimizations]))
AM_CONDITIONAL([DEBUG], [test x$enable_debug = xyes])

AC_ARG_ENABLE([loglevel],
    AS_HELP_STRING([--enable-loglevel=LOGLEVEL], [Enable logging in scalloc.
                    Levels: kTrace, kInfo, kWarning, kError, kFatal. Default:
                    kWarning]), 
    [AC_DEFINE_UNQUOTED([LOG_LEVEL], [$enableval], [log level])],
    [AC_DEFINE([LOG_LEVEL], [kWarning], [log level])]
)

AC_ARG_ENABLE([sbrkspace],
    AS_HELP_STRING([--enable-sbrkspace=SBRK_SPACE], [Enable a custom sbrk size
                    for the backend allocator]), 
    [AC_DEFINE_UNQUOTED([SBRK_SPACE], [$enableval], [sbrk space size])],
    []
)

AC_ARG_ENABLE([profiler],
    AS_HELP_STRING([--enable-profiler], [Enable allocation profiling]), 
    [AC_DEFINE([PROFILER_ON], [], [enable profiling])],
    []
)

AC_ARG_ENABLE([global-cleanup],
    AS_HELP_STRING([--enable-global-cleanup], [Enable global cleanup in remote
                    freelist]), 
    [AC_DEFINE([GLOBAL_CLEANUP], [], [global cleanup])],
    []
)

AC_ARG_ENABLE([span-reuse-threshold],
    AS_HELP_STRING([--enable-span-reuse-threshold=PERCENT], []),
    [AC_DEFINE_UNQUOTED([SPAN_REUSE_THRESHOLD], [$enableval], [reuse threshold])],
    [AC_DEFINE([SPAN_REUSE_THRESHOLD], [20], [threshold for potentially reusing spans])]
)

AC_ARG_ENABLE([local-reuse-threshold],
    AS_HELP_STRING([--enable-local-reuse-threshold=PERCENT], []),
    [AC_DEFINE_UNQUOTED([LOCAL_REUSE_THRESHOLD], [$enableval], [reuse threshold])],
    [AC_DEFINE([LOCAL_REUSE_THRESHOLD], [80], [threshold for swapping local
     active span with a potentially reusable span])]
)

dnl enable freelist bounds checks
AC_ARG_ENABLE([check-freelist],
    AS_HELP_STRING([--check-freelist], [Enable bound checks in freelist]))

AS_IF([test "x$enable_check_freelist" = "xyes"], [
  AC_DEFINE([FREELIST_CHECK_BOUNDS], [], [enable bound checks in freelist]) 
  AC_MSG_NOTICE([freelist bounds checks enabled])
])

AC_CONFIG_FILES(Makefile)

AC_OUTPUT
