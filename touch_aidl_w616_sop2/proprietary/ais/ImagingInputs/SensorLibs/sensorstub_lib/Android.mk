#======================================================================
#makefile for libais_sensorstub.so
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MY_AIS_ROOT := $(LOCAL_PATH)/../../..

LOCAL_C_INCLUDES += \
	$(MY_AIS_ROOT)/API/inc \
	$(MY_AIS_ROOT)/CameraDevice/inc \
	$(MY_AIS_ROOT)/CameraEventLog/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServices/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServicesMMOSAL/inc \
	$(MY_AIS_ROOT)/Common/inc \
	$(MY_AIS_ROOT)/ImagingInputs/ImagingInputDriver/inc \
	external/libxml2/include

LOCAL_SRC_FILES += \
	src/sensorstub_lib.c

LOCAL_CFLAGS := $(ais_compile_cflags)

LOCAL_LDFLAGS :=

ifeq ($(call is-platform-sdk-version-at-least,28),true)
#Only Android Q uses libmmsoal_proprietary
ifneq ( ,$(filter 10 Q, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
else
LOCAL_HEADER_LIBRARIES := libmmosal_headers
endif
endif

ifneq ($(AIS_BUILD_STATIC),true)
ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION))))
LOCAL_SHARED_LIBRARIES:= libais libais_log_proprietary
else
LOCAL_SHARED_LIBRARIES:= libais libais_log
endif
endif

LOCAL_SHARED_LIBRARIES += libxml2

LOCAL_MODULE := libais_sensorstub

ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION))))
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
