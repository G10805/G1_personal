#======================================================================
#libais_test_util_proprietary.so
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
	external/icu/icu4c/source/common \
	$(LOCAL_PATH)/../../../../gles/adreno200/include/private/C2D \
	$(LOCAL_PATH)/../../../../gles/adreno200/c2d30/include

LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES := src/test_util.cpp src/test_util_standalone.cpp src/la/test_util_la.cpp \
	../../CameraOSServices/CameraOSServicesMMOSAL/src/CameraOSServices.c

LOCAL_CFLAGS := -D_ANDROID_ \
	-DC2D_DISABLED \
	-Werror -Wno-unused-parameter

LOCAL_LDFLAGS :=

LOCAL_SHARED_LIBRARIES := liblog libais_log_proprietary_qcc6 \
	libxml2 libC2D2 \
	libdl libcutils libEGL libGLESv2 libui \
	libhidlbase \
	libutils

ifneq ( ,$(filter 10 Q ,$(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libgui_vendor
endif

#gralloc_priv.h location moved in Android Q and newer
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display \
        $(TOP)/frameworks/native/include \
        $(TARGET_OUT_HEADERS)/common/inc
LOCAL_CFLAGS += -DTESTUTIL_VENDOR_LIB
LOCAL_SHARED_LIBRARIES += libbinder
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_headers
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
LOCAL_SHARED_LIBRARIES += libhidlbase libgralloctypes android.hardware.graphics.mapper@4.0
ifneq ( ,$(filter 11 R ,$(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_R_AOSP
endif
else
#use proprietary libs for Android Q
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_SHARED_LIBRARIES += libmmosal_proprietary
LOCAL_CFLAGS += -DANDROID_Q_AOSP
endif
LOCAL_HEADER_LIBRARIES += display_intf_headers
LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE := libais_test_util_proprietary_qcc6

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
include $(BUILD_SHARED_LIBRARY)
endif

#======================================================================
#libais_test_util_edrm.so/libais_test_util_edrm_proprietary.so
#======================================================================
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/inc \
	$(LOCAL_PATH)/../../API/inc \
	$(TARGET_OUT_HEADERS)/qcarcam \
	$(LOCAL_PATH)/../../Common/inc \
	$(LOCAL_PATH)/../../CameraOSServices/CameraOSServices/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include \
	external/libxml2/include \
	external/icu/icu4c/source/common

# Graphic and edrm headers
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/ \
                    $(TARGET_OUT_HEADERS)/adreno \
                    $(TOP)/external/libdrm

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES := src/test_util.cpp src/la/test_util_edrm.cpp \
	../../CameraOSServices/CameraOSServicesMMOSAL/src/CameraOSServices.c

LOCAL_CFLAGS := -D_ANDROID_ -DION_NEW

LOCAL_LDFLAGS :=

LOCAL_SHARED_LIBRARIES :=  liblog libxml2 libutils libion

# Link libdrm as static to avoid system/vendor problem
LOCAL_WHOLE_STATIC_LIBRARIES  += libdrm

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := libais_test_util_edrm_proprietary_qcc6
LOCAL_SHARED_LIBRARIES += libais_log_proprietary_qcc6 libgsl libadreno_utils libEGL_adreno libGLESv2_adreno libC2D2
#Android R uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_headers
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
ifneq ( ,$(filter 11 R ,$(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DANDROID_R_AOSP
endif
else
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_SHARED_LIBRARIES += libmmosal_proprietary
endif
else
LOCAL_MODULE := libais_test_util_edrm_qcc6
LOCAL_HEADER_LIBRARIES := libmmosal_headers
LOCAL_SHARED_LIBRARIES += libais_log libgsl_early libadreno_utils_early libEGL_adreno_early libGLESv2_adreno_early libC2D2_early
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
LOCAL_CFLAGS += -DAIS_EARLYSERVICE
endif

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

#include $(BUILD_SHARED_LIBRARY)

#===================
#config files
#===================
include $(CLEAR_VARS)
LOCAL_MODULE:= 1cam_qcc6.xml
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := config/1cam.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
include $(BUILD_PREBUILT)
include $(CLEAR_VARS)
LOCAL_MODULE:= 8cam_qcc6.xml
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := config/8cam.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
include $(BUILD_PREBUILT)
include $(CLEAR_VARS)
LOCAL_MODULE:= 1cam_pproc_nv12_qcc6.xml
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := config/1cam_pproc_nv12.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
LOCAL_MODULE_OWNER := qti
include $(BUILD_PREBUILT)
