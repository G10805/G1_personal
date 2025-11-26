# common.mk - Makefile for Chi-CDK
#
# Things in this file are global to all Chi-CDK sub-projects. Consider adding things like
# include paths and linked libraries to individual sub-projects instead.
#

# Set to enable function call profiling on gcc for supported modules
# ANDed with SUPPORTS_FUNCTION_CALL_TRACE in each makefile that includes common.mk
ENABLE_FUNCTION_CALL_TRACE := 0

# CAMX_OS can be linux or win32
CAMX_OS := linux

# OEM interpolation
CAMX_OEM1IQ_PATH := $(CAMX_PATH_PREFIX)/oemiq-ss/oem1iq

# Sanity
ifeq ($(CAMX_CHICDK_PATH),)
$(error CAMX_CHICDK_PATH must be defined before calling this makefile!)
endif

# Helper functions to check for valid paths
ifeq ($(CAMX_CDK_PATH),)
    CAMX_CDK_PATH := $(CAMX_CHICDK_PATH)/cdk
endif
ifeq ($(CAMX_VENDOR_PATH),)
    CAMX_VENDOR_PATH := $(CAMX_CHICDK_PATH)/vendor
endif

ifndef CAMX_OUT_HEADERS
    CAMX_OUT_HEADERS := $(TARGET_OUT_HEADERS)/camx
endif

CAMX_C_INCLUDES :=                              \
    external/zlib                               \
    external/openssl/include                    \
    system/core/libsync/include                 \
    system/media/camera/include                 \
    $(CAMX_CDK_PATH)                            \
    $(CAMX_CDK_PATH)/chi                        \
    $(CAMX_CDK_PATH)/common                     \
    $(CAMX_CDK_PATH)/fd                         \
    $(CAMX_CDK_PATH)/isp                        \
    $(CAMX_CDK_PATH)/ncs                        \
    $(CAMX_CDK_PATH)/node                       \
    $(CAMX_CDK_PATH)/pdlib                      \
    $(CAMX_CDK_PATH)/sensor                     \
    $(CAMX_CDK_PATH)/stats                      \
    $(CAMX_CDK_PATH)/generated/g_facedetection  \
    $(CAMX_CDK_PATH)/generated/g_parser         \
    $(CAMX_CDK_PATH)/generated/g_sensor         \
    $(CAMX_OUT_HEADERS)                         \
    $(TARGET_OUT_HEADERS)

ifeq ($(TARGET_KERNEL_VERSION),$(filter 5.15 6.1,$(TARGET_KERNEL_VERSION)))
# Include path for Camera Kernel UAPI headers
ifneq ($(ANDROID_TOP_ROOT),)
    CAMERA_KERNEL_UAPI_INCLUDE_PATH := $(ANDROID_TOP_ROOT)/vendor/qcom/opensource/ais-kernel/include/uapi/ais
else
    CAMERA_KERNEL_UAPI_INCLUDE_PATH := $(QC_PROP_ROOT)/../opensource/ais-kernel/include/uapi/ais
endif
CAMX_C_INCLUDES += $(CAMERA_KERNEL_UAPI_INCLUDE_PATH)
endif

ifeq ($(IQSETTING),OEM1)
LOCAL_C_INCLUDES :=                             \
    $(CAMX_OEM1IQ_PATH)/chromatix/g_chromatix
else
LOCAL_C_INCLUDES :=                             \
    $(CAMX_CDK_PATH)/generated/g_chromatix
endif


# Always include the system paths last
CAMX_C_INCLUDES += $(CAMX_SYSTEM_INCLUDES)

# Put here any libraries that should be linked by Chi-CDK projects
CAMX_C_LIBS :=

# VGDB build specific flags that must come first as they clobber warnings
ifneq ($(CAMX_EXT_VBUILD),)
CAMX_CFLAGS :=                  \
    $(CAMX_ADDITIONAL_CFLAGS)
else
CAMX_CFLAGS :=
endif # ($(CAMX_EXT_VBUILD),)

# Common CFLags to the project
CAMX_CFLAGS +=                              \
    $(DWARN_ALWAYS_ON)                      \
    $(DLINUX_FLAGS)                         \
    $(CFLAGS)                               \
    -DCAMX                                  \
    -D_LINUX                                \
    -DCAMX_LOGS_ENABLED=1                   \
    -DCAMX_TRACES_ENABLED=1                 \
    -fPIC                                   \
    -Werror                                 \
    -Wno-missing-field-initializers         \
    -Wno-register

# Automotive specific CFlags to the project
CAMX_CFLAGS += -D__CAMX_AUTO_CHANGES__
#Workaround for Android Q compilation
ifneq ( ,$(filter 10 Q R, $(PLATFORM_VERSION)))
CAMX_CFLAGS += -Wno-unused-variable -Wno-unused-function -Wno-unused-private-field -Wno-unused-value
endif

LLVM_VER4 = $(findstring 4.0,$(SDCLANG_PATH))
ifneq (,$(strip $(LLVM_VER4)))
        CAMX_CFLAGS += -Wno-address-of-packed-member
endif


# Release builds should hide symbols
ifeq ($(CAMXDEBUG),)
CAMX_CFLAGS += -fvisibility=hidden
endif #CAMXDEBUG

ifneq ($(LOCAL_CLANG),false)
CAMX_CFLAGS += -Wno-implicit-exception-spec-mismatch
else # ($(LOCAL_CLANG),false)
# Increase function alignment for better and more repeatable performance
CAMX_CFLAGS += -falign-functions=32
endif # ($(LOCAL_CLANG),false)

# -Wdate-time only support in gcc 4.9 and above.
ifeq ($(filter 4.6 4.6.% 4.7 4.7.% 4.8 4.8.%, $(TARGET_GCC_VERSION)),)
# 8994 L release use gcc 4.9 but don't have date-time support.
ifneq ($(filter -Werror=date-time,$(TARGET_ERROR_FLAGS)),)
CAMX_CFLAGS += -Wno-error=date-time
endif # TARGET_ERROR_FLAGS
endif # TARGET_GCC_VERSION

# 64bit builds generate many type punning warnings which suggests possible bugs
# Disable strict aliasing optimizations. Note that warning cannot be suppressed with -Wno-strict-aliasing.
CAMX_CFLAGS += -fno-strict-aliasing
CAMX_CFLAGS += -fstack-protector-strong

# VGDB build specific C++ flags that must come first as they clobber warnings
ifneq ($(CAMX_EXT_VBUILD),)
CAMX_CPPFLAGS :=                  \
    $(CAMX_ADDITIONAL_CPPFLAGS)
else
CAMX_CPPFLAGS :=
endif # ($(CAMX_EXT_VBUILD),)

CAMX_CPPFLAGS +=                \
    -Wno-invalid-offsetof       \
    -std=c++17                  \
    -fstack-protector-strong

ifeq ($(CAMX_EXT_VBUILD),)
    # Linux build always uses SDLLVM
    LOCAL_SDCLANG := true

    # Force ARM mode here. The driver will build as THUMB2 in 32bit release builds but we want to
    # link with the ARM libs to match the Linux based builds. Improves run to run variation.
    LOCAL_ARM_MODE := arm
endif

# Release builds vs debug builds
ifeq ($(CAMXDEBUG),)
    # Use the highest optimization level with SDLLVM and
    # use the latest version (>=4.0)
    ifeq ($(LOCAL_SDCLANG), true)
        LOCAL_SDCLANG_OFAST := true
        SDCLANG_FLAG_DEFS := $(CAMX_VENDOR_PATH)/sdllvm-flag-defs.mk
        # Select latest version of SDLLVM (>=4.0).
        SDCLANG_VERSION_DEFS := $(CAMX_VENDOR_PATH)/sdllvm-selection.mk
        -include $(SDCLANG_VERSION_DEFS)
    endif # ($(LOCAL_SDCLANG), true)
endif # ($(CAMXDEBUG),)

# Optimization flags
ifneq ($(CAMX_EXT_VBUILD),)

ifeq ($(CAMXDEBUG),)
CAMX_CFLAGS += -O0
else # ($(CAMXDEBUG),)
CAMX_CFLAGS += -O0
endif # ($(CAMXDEBUG),)

else # ($(CAMX_EXT_VBUILD),)

ifeq ($(CAMXDEBUG),)
# @todo (CAMX-2029) Need to change this back to O3 once we fix why it fails
CAMX_CFLAGS += -O0
else # ($(CAMXDEBUG),)
CAMX_CFLAGS += -O0
endif # ($(CAMXDEBUG),)

endif # ($(CAMX_EXT_VBUILD),)

# Describes what type of thread specific objects we want to have
# By default we will use POSIX style TLS objects using pthread_key create/delete APIs.
# A few pros in using this style of TLS objects are -
#   - Complete control over the life cycle of TLS objects.
#   - Provides compiler independent code, specifically, this is
#     necessary to compile modules while using LLVM (clang) compiler
# The other option is to use the compiler dependent __thread data type.
ENABLE_PTHREAD_TLS := 1
ifeq ($(ENABLE_PTHREAD_TLS),1)
CAMX_CFLAGS += -DPTHREAD_TLS
endif

ifeq ($(ENABLE_FUNCTION_CALL_TRACE),1)
    ifeq ($(SUPPORT_FUNCTION_CALL_TRACE),1)
        # -finstrument-functions    Add calls to __cyg_profile_func_enter()/__cyg_profile_func_exit()
        #                           around function calls
        # -mpoke-function-name      Embed the name of each function above the function itself
        CAMX_CFLAGS += -finstrument-functions -mpoke-function-name -DFUNCTION_CALL_TRACE
    endif #ifeq ($(SUPPORT_FUNCTION_CALL_TRACE),1)
endif #ifeq ($(ENABLE_FUNCTION_CALL_TRACE),1)

CAMX_CFLAGS += -DCAMX_ANDROID_API=$(PLATFORM_SDK_VERSION)
CAMX_CPPFLAGS += -DCAMX_ANDROID_API=$(PLATFORM_SDK_VERSION)

# Defaults projects are unlikely to change (@todo: what is local module relative path?)
LOCAL_SHARED_LIBRARIES := libc libc++ libcutils libdl liblog
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH :=

# These flags prevent using the PLT for looking up imports (they are resolved
# at build time instead). This directly affects code size and driver overhead.
# The flags used to be default in the Android system makefiles. Since they
# have a large impact on driver overhead performance, ensure they are used
# regardless of whether the Android makefiles enable them.
LOCAL_LDFLAGS += -Wl,-shared,-Bsymbolic

# For __android_log
LOCAL_LDLIBS += -llog

LOCAL_HEADER_LIBRARIES :=

ifneq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
CAMX_CFLAGS += -DANDROID_Q_AOSP -DCAMX_USE_GRALLOC1
endif

# P and above
ifeq ($(call CHECK_VERSION_GE, $(PLATFORM_SDK_VERSION), $(PLATFORM_SDK_PPDK)), true)
LOCAL_HEADER_LIBRARIES += libutils_headers
LOCAL_HEADER_LIBRARIES += libcutils_headers
LOCAL_HEADER_LIBRARIES += libhardware_headers
endif

LOCAL_MODULE_PATH_32 := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_VENDOR)/lib64

CAMX_LIB_OUTPUT_PATH := camera/components
CAMX_BIN_OUTPUT_PATH := camera
