#======================================================================
#makefile for libais.so
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MY_AIS_ROOT := $(LOCAL_PATH)
CHI_CDK_PATH := $(LOCAL_PATH)/../../chi-cdk

LOCAL_C_INCLUDES += \
	$(MY_AIS_ROOT)/API/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServices/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServicesMMOSAL/inc \
	$(MY_AIS_ROOT)/CameraQueue/CameraQueue/inc \
	$(MY_AIS_ROOT)/Common/inc \
	$(MY_AIS_ROOT)/Engine/inc \
	$(MY_AIS_ROOT)/CameraEventLog/inc \
	$(MY_AIS_ROOT)/CameraMulticlient/common/inc \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/ais \
	$(TARGET_OUT_HEADERS)/adreno \
	$(TARGET_OUT_HEADERS)/mm-osal/include \
	$(TARGET_OUT_HEADERS)/common/inc \
	$(TARGET_OUT_HEADERS)/qcom/display \
	system/media/camera/include

ifeq ($(TARGET_KERNEL_VERSION), 4.14)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
endif

LOCAL_C_INCLUDES+= \
	$(MY_AIS_ROOT)/Engine/pproc \
	$(MY_AIS_ROOT)/Engine/pproc/chi \
	$(MY_AIS_ROOT)/Engine/pproc/chi/inc \
	$(MY_AIS_ROOT)/PostProc/inc \
	$(CHI_CDK_PATH)/cdk/chi \
	$(CHI_CDK_PATH)/cdk \
	$(CHI_CDK_PATH)/cdk/isp \
	$(CHI_CDK_PATH)/cdk/common \
	$(CHI_CDK_PATH)/cdk/pdlib \
	$(CHI_CDK_PATH)/cdk/stats

CAMX_PRODUCT := msmnile
ifneq ( ,$(filter $(MSMSTEPPE) $(MSMSTEPPE)_au, $(TARGET_BOARD_PLATFORM)))
CAMX_PRODUCT := $(MSMSTEPPE)
endif

LOCAL_C_INCLUDES+= \
	$(CHI_CDK_PATH)/vendor/chioverride/default/$(CAMX_PRODUCT)/

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_C_INCLUDES += \
	$(MY_AIS_ROOT)/CameraConfig/inc \
	$(MY_AIS_ROOT)/CameraDevice/inc \
	$(MY_AIS_ROOT)/CameraPlatform/inc \
	$(MY_AIS_ROOT)/CameraPlatform/linux \
	$(MY_AIS_ROOT)/HWDrivers/API \
	$(MY_AIS_ROOT)/HWDrivers/API/IFEDriver \
	$(MY_AIS_ROOT)/HWDrivers/API/MIPICSIDriver \
	$(MY_AIS_ROOT)/HWDrivers/Linux/CSIPHYDriver \
	$(MY_AIS_ROOT)/HWDrivers/Linux/IFEDriver \
	$(MY_AIS_ROOT)/ImagingInputs/ImagingInputDriver/inc

LOCAL_SRC_DIR :=\
	$(MY_AIS_ROOT)/Engine/src \
	$(MY_AIS_ROOT)/Engine/pproc \
	$(MY_AIS_ROOT)/Engine/pproc/chi \
	$(MY_AIS_ROOT)/Engine/pproc/chi/src \
	$(MY_AIS_ROOT)/CameraDevice/src \
	$(MY_AIS_ROOT)/CameraEventLog/src \
	$(MY_AIS_ROOT)/CameraQueue/CameraQueueSCQ/src \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServicesMMOSAL/src \
	$(MY_AIS_ROOT)/CameraPlatform/linux \
	$(MY_AIS_ROOT)/HWDrivers/Linux/CSIPHYDriver \
	$(MY_AIS_ROOT)/HWDrivers/Linux/IFEDriver \
	$(MY_AIS_ROOT)/ImagingInputs/ImagingInputDriver/src \
	$(MY_AIS_ROOT)/ImagingInputs/ImagingInputDriver/src/linux

LOCAL_CFLAGS := $(ais_compile_cflags) \
	-D_LINUX \
	-DCAMX_ANDROID_API=28 \
	-DALOG_ENABLED \
	-Wno-unused-parameter

ifeq ($(AIS_BUILD_WITH_CAMX),true)
LOCAL_CFLAGS += -DAIS_WITH_CAMX
endif

ifeq ($(AIS_BUILD_WITH_HAL_CAMERA),true)
LOCAL_CFLAGS += -DAIS_WITH_HAL_CAMERA
endif

LOCAL_C_INCLUDES += $(LOCAL_SRC_DIR)

LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -maxdepth 1 -name '*.cpp' | sed s:^$(LOCAL_PATH)::g )


LOCAL_LDFLAGS :=

LOCAL_CPPFLAGS +=

ifeq ($(call is-platform-sdk-version-at-least,28),true)
#Only Android Q require proprietary version
ifneq ( ,$(filter 10 Q, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_SHARED_LIBRARIES := libmmosal_proprietary liblog libcutils
else
LOCAL_HEADER_LIBRARIES := libmmosal_headers
LOCAL_SHARED_LIBRARIES := libmmosal liblog libcutils
endif
endif

ifeq ($(AIS_BUILD_STATIC),true)
ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_WHOLE_STATIC_LIBRARIES  += libais_log
else
LOCAL_WHOLE_STATIC_LIBRARIES  += libais_log_proprietary
endif
LOCAL_WHOLE_STATIC_LIBRARIES  += libais_config libais_max9296 libais_max9296b
else
ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_SHARED_LIBRARIES += libais_log
else
LOCAL_SHARED_LIBRARIES += libais_log_proprietary
endif
endif

#for ais_pproc_gpu
ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_SHARED_LIBRARIES += libC2D2
else
LOCAL_SHARED_LIBRARIES += libC2D2_early
endif

LOCAL_MODULE := libais

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
endif

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifeq ($(AIS_BUILD_STATIC),true)
include $(BUILD_STATIC_LIBRARY)
else
include $(BUILD_SHARED_LIBRARY)
endif

