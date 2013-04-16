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

dnl Example of default-disabled feature
AC_ARG_ENABLE([check-freelist],
    AS_HELP_STRING([--check-freelist], [Enable bound checks in freelist]))

AS_IF([test "x$enable_check_freelist" = "xyes"], [
  AC_DEFINE([FREELIST_CHECK_BOUNDS], [], [enable bound checks in freelist]) 
  AC_MSG_NOTICE([freelist bounds checks enabled])
])

AC_CONFIG_FILES(Makefile)

AC_OUTPUT
