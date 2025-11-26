#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for qap
#

# Process this file with autoconf to produce a configure script

# Requires autoconf tool later than 2.61
AC_PREREQ(2.69)
# Initialize the ar_util package version 1.0.0
AC_INIT([ar_util],1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_PROG_CC
AC_PROG_LIBTOOL
AM_PROG_AR

AC_ARG_WITH([openwrt],
    AS_HELP_STRING([use openwrt (default is no)]),
    [with_openwrt=$withval],
    [with_openwrt=no])
AM_CONDITIONAL([BUILDSYSTEM_OPENWRT], [test "x${with_openwrt}" = "xyes"])

AC_CONFIG_FILES([ Makefile ar_util.pc ])
AC_OUTPUT
