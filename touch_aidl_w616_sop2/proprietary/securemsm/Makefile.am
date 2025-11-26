# Makefile.am - Automake script for securemsm
# LV_BUILD is used for conditional compilation of Linux Vehicles.

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = km_lite/tests

if !LV_BUILD
SUBDIRS += rpmbClient
endif

if TARGET_ENABLE_QSEECOM
CFLAGS += TARGET_ENABLE_QSEECOM
SUBDIRS += sampleclient daemon
endif

if TARGET_ENABLE_QSEECOMCOMPAT
CFLAGS += TARGET_ENABLE_QSEECOMCOMPAT
SUBDIRS += sampleclient daemon
endif

if TARGET_ENABLE_SMCI_SYSLISTENER
SUBDIRS += daemon
endif


if TRUSTED_UI
SUBDIRS += TrustedUI/TrustedVM/Composer/LayoutManager \
           TrustedUI/TrustedVM/libTrustedUI
endif

if LV_BUILD
SUBDIRS += qapi/build \
           GPTEE_Sample_client
endif

# overriding SUBDIRS for MBB builds.
if MBB_BUILD
if TARGET_ENABLE_QSEECOM
override SUBDIRS = sampleclient daemon
endif
endif

export LOAD_CMNLIB
export ENABLE_HYP

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = securemsm.pc
EXTRA_DIST = $(pkgconfig_DATA)

