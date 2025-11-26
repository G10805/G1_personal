#======================================================================
#libais_test_util.so
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../../API/inc \
    $(TARGET_OUT_HEADERS)/qcarcam \
    $(LOCAL_PATH)/../../Common/inc \
    $(LOCAL_PATH)/../../CameraOSServices/CameraOSServices/inc \
    $(TARGET_OUT_HEADERS)/mm-osal/include \
    $(TARGET_OUT_HEADERS)/qcom/display \
    external/libxml2/include \
    external/icu/icu4c/source/common

LOCAL_HEADER_LIBRARIES := libmmosal_headers

LOCAL_SRC_FILES := src/test_util.cpp src/test_util_standalone.cpp src/la/test_util_la.cpp \
    ../../CameraOSServices/CameraOSServicesMMOSAL/src/CameraOSServices.c \
    ../../Common/src/qcxlog.c

LOCAL_CFLAGS := -D_ANDROID_ \
    -DC2D_DISABLED \
    -Wno-error -Wno-unused-parameter

LOCAL_LDFLAGS :=

LOCAL_SHARED_LIBRARIES := libmmosal liblog libais_log_qcx\
    libxml2 \
    libdl libcutils libEGL libGLESv2 libui  \
    libutils libgui

#gralloc_priv.h location moved in Android Q and newer
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display \
    $(TOP)/frameworks/native/include \
    $(TARGET_OUT_HEADERS)/common/inc
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_HEADER_LIBRARIES += display_intf_headers

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_U_AOSP_ABOVE
endif

ifneq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_Q_AOSP
else
ifneq ( ,$(filter 11 R ,$(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_R_AOSP
endif
LOCAL_SHARED_LIBRARIES += libhidlbase libgralloctypes android.hardware.graphics.mapper@4.0
endif

ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../API/inc
endif

LOCAL_MODULE := libqcx_test_util

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
