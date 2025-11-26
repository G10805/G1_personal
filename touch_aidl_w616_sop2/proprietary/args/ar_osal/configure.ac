#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for qap
#

# Process this file with autoconf to produce a configure script

# Requires autoconf tool later than 2.61
AC_PREREQ(2.69)
# Initialize the ar_osal package version 1.0.0
AC_INIT([ar_osal],1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_PROG_CC
AC_PROG_LIBTOOL
AM_PROG_AR

AC_ARG_WITH(sanitized-headers,
        AS_HELP_STRING([--with-sanitized-headers=DIR],
                [Specify the location of the sanitized Linux headers]),
        [CPPFLAGS="$CPPFLAGS -I $withval"])

AC_ARG_WITH(ion,
        AS_HELP_STRING([--with-ion=DIR],
                [Specify the location of the ion headers]),
        [CPPFLAGS="$CPPFLAGS -I $withval"])

AC_ARG_WITH(audio-headers,
        AS_HELP_STRING([--with-audio-headers=DIR],
                [Specify the location of the audio kernel headers]),
        [CPPFLAGS="$CPPFLAGS -I $withval"])

AC_ARG_WITH([openwrt],
    AS_HELP_STRING([use openwrt (default is no)]),
    [with_openwrt=$withval],
    [with_openwrt=no])
AM_CONDITIONAL([BUILDSYSTEM_OPENWRT], [test "x${with_openwrt}" = "xyes"])

AC_ARG_WITH([libion],
    AS_HELP_STRING([use libion (default is yes)]),
    [with_libion=$withval],
    [with_libion=yes])
AM_CONDITIONAL([LIBION_SUPPORTED], [test "x${with_libion}" = "xyes"])

AC_ARG_WITH([glib],
      AC_HELP_STRING([--with-glib],
         [enable glib, Build against glib. Use this when building for HLOS systems which use glib]))
if (test "x${with_glib}" = "xyes"); then
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GThread >= 2.16 is required))
        PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GLib >= 2.16 is required))
        GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
        GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

        AC_SUBST(GLIB_CFLAGS)
        AC_SUBST(GLIB_LIBS)
fi
AM_CONDITIONAL(USE_GLIB, test "x${with_glib}" = "xyes")

AC_CONFIG_FILES([ Makefile ar_osal.pc ])
AC_OUTPUT
