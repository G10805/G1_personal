AC_PREREQ(2.69)
AC_INIT([fastcv],1.0.0)
AM_INIT_AUTOMAKE([-Wall gnu foreign subdir-objects])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Check for programs
AM_PROG_AS
AM_PROG_AR
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_ARG_ENABLE([arm-native-compilation],
              AC_HELP_STRING([--enable-arm-native-compilation], [Used to check if compilation happens in arm machine]),
              [arm_native="${enableval}"],
              [arm_native=no])
AM_CONDITIONAL(IS_ARM_NATIVE, test "x$arm_native" = "xyes")

AC_ARG_ENABLE(asan,
      AC_HELP_STRING([--enable-asan], [DISTRO enable asan feature.]),
         [enable_asan=${enableval}], [enable_asan="no"])
AM_CONDITIONAL(ENABLE_ASAN, [test "x${enable_asan}" = "xyes"])

AC_CONFIG_FILES([ \
        Makefile \
        samples/nsp-simple-test/Makefile \
        samples/simple-test/Makefile \
        samples/hetero-compute-test/Makefile \
        ])
AC_OUTPUT
