#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([hyp-video], [1.0], [HYPVIDEO-3])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign -Wno-portability 1.11 no-dist-gzip dist-xz tar-ustar subdir-objects])

LT_PREREQ([2.2.6])
LT_INIT

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcutils':
AC_CHECK_LIB([cutils], [main])
# FIXME: Replace `main' with a function in `-llog':
AC_CHECK_LIB([log], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h memory.h stdint.h stdlib.h string.h sys/ioctl.h unistd.h])

AC_ARG_ENABLE([support-dmabufheap],
    AC_HELP_STRING([--enable-support-dmabufheap],
    [Enable conditional compile for supporting dmabufheap [default=no]]),
    [support_dmabufheap="${enableval}"])

AM_CONDITIONAL(SUPPORT_DMABUF_HEAP, [test "x$support_dmabufheap" = "xyes"])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap])

AC_CONFIG_FILES([Makefile
                 be/Makefile
                 be/linux/Makefile
                 fe/Makefile
                 fe/linux/Makefile
                 test/Makefile
                 test/fe/Makefile
                 test/fe/linux/Makefile
                 hyp-video-intercept/Makefile])
AC_OUTPUT
