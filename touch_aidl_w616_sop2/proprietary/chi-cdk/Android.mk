PLATFORM_SDK_PPDK = 28
# Map SDK/API level to Android verisons
PLATFORM_SDK_OPDK = 26
PLATFORM_SDK_PPDK = 28
PLATFORM_SDK_QPDK = 29

# Abstract the use of PLATFORM_VERSION to detect the version of Android.
# On the Linux NDK, a build error is seen when a character such as 'Q'
#it must be an integer. On Windows NDK it may be an integer or a character.
# Linux AU builds provide a character for this variable.
# Therefore, use ANDROID_FLAVOR for everything. ANDROID_FLAVOR is defined
# in the build configuration files for the NDK environments. For Linux
# AUs, map value received from the environment to our own internal
# ANDROID_FLAVOR.

ifeq ($(ANDROID_FLAVOR_P),)
ANDROID_FLAVOR_P = P
endif # ($(ANDROID_FLAVOR_P),)

ifeq ($(ANDROID_FLAVOR_Q),)
ANDROID_FLAVOR_Q = Q
endif # ($(ANDROID_FLAVOR_Q),)

# HWASAN enablement causing build failures, as the sdclang not aware
# of the options added by HWASAN, which causing build failure.
LOCAL_NOSANITIZE := hwaddress
LOCAL_SANITIZE := never
SANITIZE_TARGET :=

ifeq ($(CAMX_EXT_VBUILD),)
    ifeq ($(PLATFORM_VERSION),$(filter $(PLATFORM_VERSION), 9 P))
        ifeq ($(ANDROID_FLAVOR),)
              ANDROID_FLAVOR := $(ANDROID_FLAVOR_P)
        endif # ($(ANDROID_FLAVOR),)
    endif # ($(PLATFORM_VERSION),$(filter $(PLATFORM_VERSION), 9 P))

    ifeq ($(PLATFORM_VERSION),$(filter $(PLATFORM_VERSION), 10 Q))
       ifeq ($(ANDROID_FLAVOR),)
             ANDROID_FLAVOR := $(ANDROID_FLAVOR_Q)
       endif # ($(ANDROID_FLAVOR),)
    endif # ($(PLATFORM_VERSION),$(filter $(PLATFORM_VERSION), 10 Q))
endif # ($(CAMX_EXT_LNDKBUILD),)

#Determine the proper SDK/API level for the given Android version
ifeq ($(PLATFORM_SDK_VERSION),)
    ifeq ($(ANDROID_FLAVOR), $(ANDROID_FLAVOR_P))
        PLATFORM_SDK_VERSION := $(PLATFORM_SDK_PPDK)
    endif # ($(ANDROID_FLAVOR), $(ANDROID_FLAVOR_P))

    ifeq ($(ANDROID_FLAVOR), $(ANDROID_FLAVOR_Q))
        PLATFORM_SDK_VERSION := $(PLATFORM_SDK_QPDK)
    endif # ($(ANDROID_FLAVOR), $(ANDROID_FLAVOR_Q))
endif # ($(PLATFORM_SDK_VERSION),)

CAMX_CHICDK_PATH := $(call my-dir)
CAMX_CDK_PATH := $(CAMX_CHICDK_PATH)/cdk
CAMX_VENDOR_PATH := $(CAMX_CHICDK_PATH)/vendor

# Take backup of SDLLVM specific flag and version defs as other modules (adreno)
# also maintain their own version of it.
OLD_SDCLANG_FLAG_DEFS    := $(SDCLANG_FLAG_DEFS)
OLD_SDCLANG_VERSION_DEFS := $(SDCLANG_VERSION_DEFS)

include $(CAMX_CDK_PATH)/build/android/autogen.mk
include $(CAMX_CDK_PATH)/generated/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/Android.mk

ifeq ($(PLATFORM_VERSION),$(filter $(PLATFORM_VERSION), 10 Q))
include $(CAMX_CHICDK_PATH)/test/camx-hal3-test/Android.mk
endif

# Restore previous value of sdllvm flag and version defs
SDCLANG_FLAG_DEFS    := $(OLD_SDCLANG_FLAG_DEFS)
SDCLANG_VERSION_DEFS := $(OLD_SDCLANG_VERSION_DEFS)
