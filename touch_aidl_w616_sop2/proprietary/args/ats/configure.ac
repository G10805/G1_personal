#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for qap
#

# Process this file with autoconf to produce a configure script

# Requires autoconf tool later than 2.61
AC_PREREQ(2.69)
# Initialize the ats package version 1.0.0
AC_INIT([ats],1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([-Wall -Werror gnu foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

PKG_PROG_PKG_CONFIG

AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AM_PROG_AR

AC_ARG_WITH([diag],
      AC_HELP_STRING([--with-diag],
         [Include diag, Include diag header files when compiling ats module]))

AC_ARG_WITH([glib],
      AC_HELP_STRING([--with-glib],
         [enable glib, Build against glib. Use this when building for HLOS systems which use glib]))

DIAG_CFLAGS=
AS_IF([test "x$with_diag" != "xno"], [DIAG_CFLAGS="-I$with_diag"])

AC_SUBST(DIAG_CFLAGS)

AC_ARG_WITH([libdiag],
    AS_HELP_STRING([use libdiag (default is no)]),
    [with_libdiag=$withval],
    [with_libdiag=no])
AM_CONDITIONAL([LIBDIAG_SUPPORTED], [test "x${with_libdiag}" = "xyes"])

 AC_ARG_WITH([common_includes],
       AC_HELP_STRING([--with-common-includes=@<:@dir@:>@],
          [Specify the location of the common headers]),
       [common_incdir=$withval],
       with_common_includes=no)

if test "x$with_common_includes" != "xno"; then
    CFLAGS="${CFLAGS} -I${common_incdir}"
fi

AC_ARG_WITH([kernel],
    AC_HELP_STRING([--with-kernel=@<:@dir@:>@],
                [Specify the location of the Linux kernel headers]),
                [kerneldir=$withval],
                with_kernel=no)

if test "x$with_kernel" != "xno"; then
    CFLAGS="${CFLAGS} -I${kerneldir}/include -I${kerneldir}/arch/arm/include"
fi

if (test "x${with_glib}" = "xyes"); then
    AC_DEFINE(ENABLE_USEGLIB, 1, [Define if HLOS systems uses glib])
    PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                AC_MSG_ERROR(GThread >= 2.16 is required))
    PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                AC_MSG_ERROR(GLib >= 2.16 is required))
    GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
    GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

    AC_SUBST(GLIB_CFLAGS)
    AC_SUBST(GLIB_LIBS)
fi

AC_ARG_WITH([openwrt],
    AS_HELP_STRING([use openwrt (default is no)]),
    [with_openwrt=$withval],
    [with_openwrt=no])
AM_CONDITIONAL([BUILDSYSTEM_OPENWRT], [test "x${with_openwrt}" = "xyes"])

AC_SUBST([CFLAGS])
AC_SUBST([CC])
AC_CONFIG_FILES([ Makefile ats.pc ])
AC_OUTPUT
