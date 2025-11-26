#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for auto test
#

# Process this file with autoconf to produce a configure script

# Requires autoconf tool later than 2.61
AC_PREREQ(2.61)
# Initialize the auto test package version 1.0.0
AC_INIT([autotest],1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign])
# Disables auto rebuilding of configure, Makefile.ins
AM_MAINTAINER_MODE
# defines some macros variable to be included by source
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_ARG_WITH([core_includes],
    AC_HELP_STRING([--with-core-includes=@<:@dir@:>@],
       [Specify the location of the core headers]),
    [core_incdir=$withval],
    with_core_includes=no)

if test "x$with_core_includes" != "xno"; then
   CFLAGS="${CFLAGS} -I${core_incdir}"
fi

AC_ARG_WITH(sanitized-headers,
   AS_HELP_STRING([--with-sanitized-headers=DIR],
       [Specify the location of the sanitized Linux headers]),
   [CPPFLAGS="$CPPFLAGS -I$withval"])

AC_ARG_ENABLE(debug,
   AS_HELP_STRING([--enable-debug],
       [For debug build variant, enable more debug features]),
   [case "${enableval}" in
       yes) debug=true ;;
       no)  debug=false ;;
       *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
   esac],[debug=false])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])
AC_SUBST([CFLAGS])
AC_SUBST([CC])
AC_CONFIG_FILES([ \
        Makefile
        test-apps/drm-test/Makefile \
        ])
AC_OUTPUT
