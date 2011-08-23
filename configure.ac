#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# $Id$
# $URL$

AC_PREREQ(2.59)
AC_INIT([frog], [0.12.4], [timbl@uvt.nl])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([config.h])

# AC_DEFINE_DIR is defined in acinclude.m4
AC_DEFINE_DIR([SYSCONF_PATH], sysconfdir, sysconfdir)

if test x"${CXXFLAGS+set}" = xset; then
  # the user set CXXFLAGS; don't override it.
  cxx_flags_were_set=true
else
  cxx_flags_were_set=false
fi

# Checks for programs.
AC_PROG_CXX( [g++] )

if $cxx_flags_were_set; then
  CXXFLAGS=$CXXFLAGS
fi

AC_PROG_RANLIB
# use libtool
AC_PROG_LIBTOOL
AC_SUBST([LIBTOOL_DEPS])

# when running tests, use CXX
AC_LANG([C++])

ACX_PTHREAD([],[AC_MSG_ERROR([We need pthread support!])])

if test x"$acx_pthread_ok" = xyes; then
   LIBS="$PTHREAD_LIBS $LIBS" \
   CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
fi

# check for OpenMP
OPENMP_CHECK( [CXXFLAGS="$CXXFLAGS $OPENMP_CXXFLAGS"],
	      [AC_MSG_ERROR([We need OpenMP, please use a recent compiler!])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_C_INLINE
AC_C_CONST
AC_TYPE_INT32_T
AC_TYPE_SIZE_T

# Checks for libraries.

if test $prefix = "NONE"; then
   prefix="$ac_default_prefix"
fi

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_STAT
AC_CHECK_FUNCS([memset])

PKG_PROG_PKG_CONFIG
AC_PATH_PROG(pkgconfigpath, pkg-config, NONE)
if test "$pkgconfigpath" != "NONE"; then
# ugly hack when PKG_CONFIG_PATH isn't defined.
# couldn't get it to work otherwise
  if test "x$PKG_CONFIG_PATH" = x; then
     export PKG_CONFIG_PATH=""
  fi
fi

#check for needed dependencies
AC_ARG_WITH(timbl,
       [  --with-timbl=DIR       use timbl installed in <DIR>],
       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$withval/lib/pkgconfig"],
       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$prefix/lib/pkgconfig"])
PKG_CHECK_MODULES([timbl], [timbl >= 6.4.1] )
CXXFLAGS="$CXXFLAGS $timbl_CFLAGS"
LIBS="$LIBS $timbl_LIBS"

AC_ARG_WITH(timblserver,
       [  --with-timblserver=DIR     use timblserver installed in <DIR>],
       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$withval/lib/pkgconfig"],
       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$prefix/lib/pkgconfig"])
PKG_CHECK_MODULES([timblserver], [timblserver >= 1.2] )
CXXFLAGS="$CXXFLAGS $timblserver_CFLAGS"
LIBS="$LIBS $timblserver_LIBS"

AC_ARG_WITH(mbt,
       [  --with-mbt=DIR       use mbt installed in <DIR>],
       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$withval/lib/pkgconfig"],
       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$prefix/lib/pkgconfig"])
PKG_CHECK_MODULES([mbt], [mbt >= 3.2.6] )
CXXFLAGS="$CXXFLAGS $mbt_CFLAGS"
LIBS="$LIBS $mbt_LIBS"

AC_ARG_WITH(ucto,
       [  --with-ucto=DIR       use ucto installed in <DIR>],
       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$withval/lib/pkgconfig"],
       [PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$prefix/lib/pkgconfig"])
PKG_CHECK_MODULES([ucto], [ucto >= 0.4.7] )
CXXFLAGS="$CXXFLAGS $ucto_CFLAGS"
LIBS="$LIBS $ucto_LIBS"

# needed till icu 4.6 is widely available
PKG_CHECK_MODULES( [icu], [ucto-icu >= 3.6] )
CXXFLAGS="$CXXFLAGS $icu_FLAGS"
LIBS="$LIBS $icu_LIBS"

#
# the ac_python_devel.m4 script tramples upon $LIBS, so we save it
save_LIBS="$LIBS"
save_FLAGS="$CXXFLAGS"

AC_ARG_WITH(python,
       [  --with-python=DIR       use python executable installed in <DIR>],
       [PYTHON="$withval/python"],
       [PYTHON="python"])

#we need Python > 2.5.x
AM_PATH_PYTHON([2.5],
	[AC_DEFINE_DIR([PYTHONDIR],pkgpythondir, PYTHONDIR)],
	[:])

# set PYTHON_EXTRA_LIBS to a non-empty string
# this fools AC_PYTHON_DEVEL into NOT setting it.
# That fails on some systems with linking errors
# we don't need PYTHON_EXTRA_LIBS
PYTHON_EXTRA_LIBS=" "

AC_PYTHON_DEVEL([>= '2.5'])
if test "x$PYTHON_CPPFLAGS" != "x"; then
 CXXFLAGS="$save_FLAGS $PYTHON_CPPFLAGS"
 LIBS="$save_LIBS $PYTHON_LDFLAGS"
else
 CXXFLAGS="$save_FLAGS"
 LIBS="$save_LIBS"
fi

AC_CHECK_HEADERS( [Python.h], , AC_MSG_FAILURE("can't find Python C API") )

AC_OUTPUT([
  Makefile
  m4/Makefile
  config/Makefile
  docs/Makefile
  scripts/Makefile
  src/Makefile
  include/Makefile
  include/frog/Makefile
])