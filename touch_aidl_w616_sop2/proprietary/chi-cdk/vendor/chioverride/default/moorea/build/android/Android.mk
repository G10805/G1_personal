ifeq ($(CAMX_CHICDK_PATH),)
LOCAL_PATH := $(abspath $(call my-dir)/../../..)
CAMX_CHICDK_PATH := $(abspath $(LOCAL_PATH)/../../..)
else
LOCAL_PATH := $(CAMX_CHICDK_PATH)/vendor/chioverride/default
endif

include $(CLEAR_VARS)

# Module supports function call tracing via ENABLE_FUNCTION_CALL_TRACE
# Required before including common.mk
SUPPORT_FUNCTION_CALL_TRACE := 1

# Get definitions common to the CAMX project here
include $(CAMX_CHICDK_PATH)/vendor/common.mk

LOCAL_INC_FILES :=                 \
    cdkutils.h                     \
    chxadvancedcamerausecase.h     \
    chxdefs.h                      \
    chxextensionmodule.h           \
    chxfeature.h                   \
    chxfeaturehdr.h                \
    chxfeaturemultiframe.h         \
    chxfeaturemfnr.h               \
    chxfeaturemfsr.h               \
    chxfeaturequadcfa.h            \
    chxfeaturerawjpeg.h            \
    chxfeatureyuvcb.h              \
    chxfeaturezsl.h                \
    chxincs.h                      \
    chxmetadata.h                  \
    chxmulticamcontroller.h        \
    chxpipeline.h                  \
    chxsensorselectmode.h          \
    chxsession.h                   \
    chxusecase.h                   \
    chxusecasedefault.h            \
    chxusecasemc.h                 \
    chxusecasevrmc.h               \
    chxusecasequadcfa.h            \
    chxusecasesuperslowmotionfrc.h \
    chxusecasetorch.h              \
    chxusecaseutils.h              \
    chxutils.h                     \
    chxzoomtranslator.h            \
    chxperf.h                      \
    moorea/g_pipelines.h

LOCAL_SRC_FILES :=                      \
    chxadvancedcamerausecase.cpp        \
    chxextensioninterface.cpp           \
    chxextensionmodule.cpp              \
    chxfeature.cpp                      \
    chxfeaturehdr.cpp                   \
    chxfeaturemultiframe.cpp            \
    chxfeaturemfnr.cpp                  \
    chxfeaturemfsr.cpp                  \
    chxfeaturequadcfa.cpp               \
    chxfeaturerawjpeg.cpp               \
    chxfeatureyuvcb.cpp                 \
    chxfeaturezsl.cpp                   \
    chxmetadata.cpp                     \
    chxmulticamcontroller.cpp           \
    chxpipeline.cpp                     \
    chxsensorselectmode.cpp             \
    chxsession.cpp                      \
    chxusecase.cpp                      \
    chxusecasedefault.cpp               \
    chxusecasemc.cpp                    \
    chxusecasevrmc.cpp                  \
    chxusecasequadcfa.cpp               \
    chxusecasesuperslowmotionfrc.cpp    \
    chxusecasetorch.cpp                 \
    chxusecaseutils.cpp                 \
    chxutils.cpp                        \
    chxperf.cpp                         \
    chxzoomtranslator.cpp

ifeq ($(call CHECK_VERSION_GE, $(PLATFORM_SDK_VERSION), $(PLATFORM_SDK_PPDK)), true)
LOCAL_INC_FILES +=               \
    chxusecasedual.h
LOCAL_SRC_FILES +=               \
    chxusecasedual.cpp
endif

LOCAL_C_LIBS := $(CAMX_C_LIBS)

LOCAL_C_INCLUDES :=                          \
    $(CAMX_C_INCLUDES)                       \
    $(CAMX_CDK_PATH)/utils                   \
    hardware/libhardware/include/hardware    \
    hardware/qcom/display/libgralloc         \
    hardware/qcom/display/gralloc            \
    system/core/libsync/include              \
    system/core/include                      \
    system/media/camera/include              \
    system/media/private/camera/include      \
    $(LOCAL_PATH)/moorea

# Compiler flags
LOCAL_CFLAGS := $(CAMX_CFLAGS)
LOCAL_CPPFLAGS := $(CAMX_CPPFLAGS)

LOCAL_SHARED_LIBRARIES +=           \
    libcamera_metadata              \
    libcutils                       \
    libhardware                     \
    libhidlbase                     \
    liblog                          \
    libsync                         \
    libutils                        \
    vendor.qti.hardware.vpp@1.1     \
    vendor.qti.hardware.vpp@1.2

ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES +=           \
    android.hardware.graphics.allocator@4.0             \
    android.hardware.graphics.mapper@4.0                \
    libgralloc.qti                                      \
    libgralloctypes                                     \
    vendor.qti.hardware.display.allocator@4.0           \
    vendor.qti.hardware.display.mapper@4.0              \
    vendor.qti.hardware.display.mapperextensions@1.1

LOCAL_HEADER_LIBRARIES := display_intf_headers display_headers
else
LOCAL_SHARED_LIBRARIES += libqdMetaData
endif

ifeq ($(PLATFORM_SDK_VERSION),$(PLATFORM_SDK_PPDK))
LOCAL_SHARED_LIBRARIES +=           \
    libhidltransport
endif

LOCAL_HEADER_LIBRARIES +=           \
    display_headers                 \

# Libraries to link
# Build in vendor partition
LOCAL_PROPRIETARY_MODULE := true

# Deployment path under lib/lib64
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE := com.qti.chi.override_moorea

include $(BUILD_SHARED_LIBRARY)
