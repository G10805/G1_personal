#======================================================================
#makefile for libais_config.so
#======================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MY_AIS_ROOT := $(LOCAL_PATH)

ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
ifneq (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_LDFLAGS := -Wl,-rpath,'/vendor_early_services/system/lib64' -Wl,--dynamic-linker,/vendor_early_services/system/bin/bootstrap/linker64
endif
endif

LOCAL_C_INCLUDES += \
	$(MY_AIS_ROOT)/API/inc \
	$(MY_AIS_ROOT)/CameraConfig/inc \
	$(MY_AIS_ROOT)/CameraDevice/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServices/inc \
	$(MY_AIS_ROOT)/CameraPlatform/inc \
	$(MY_AIS_ROOT)/CameraPlatform/linux \
	$(MY_AIS_ROOT)/Common/inc \
	$(MY_AIS_ROOT)/ImagingInputs/ImagingInputDriver/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include

LOCAL_SRC_DIR := \
	$(MY_AIS_ROOT)/CameraConfig/src

LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -maxdepth 1 -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_CFLAGS := $(ais_compile_cflags) \
	-Wno-unused-parameter

LOCAL_LDFLAGS :=

ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION))))
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
else
LOCAL_HEADER_LIBRARIES := libmmosal_headers
endif

#Android R onwards uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_headers
endif

ifneq ($(AIS_BUILD_STATIC),true)
ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION))))
LOCAL_SHARED_LIBRARIES:= libais libais_log_proprietary
else
LOCAL_SHARED_LIBRARIES:= libais libais_log
endif
endif

#only enable xml parser if not statically linking
ifneq ($(AIS_BUILD_STATIC),true)
LOCAL_C_INCLUDES += external/libxml2/include
LOCAL_CFLAGS += -DCAMERA_CONFIG_ENABLE_XML_PARSER
LOCAL_SHARED_LIBRARIES+= libxml2
endif

LOCAL_MODULE := libais_config

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifeq ($(AIS_BUILD_STATIC),true)
include $(BUILD_STATIC_LIBRARY)
else
include $(BUILD_SHARED_LIBRARY)
endif

