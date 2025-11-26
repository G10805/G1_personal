#======================================================================
#makefile for ais_server
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

MY_AIS_ROOT := $(LOCAL_PATH)

ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
ifneq (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_LDFLAGS := -Wl,-rpath,'/vendor_early_services/system/lib64' -Wl,--dynamic-linker,/vendor_early_services/system/bin/bootstrap/linker64
endif
else
LOCAL_LDFLAGS :=
endif

LOCAL_SRC_FILES:= \
	CameraMulticlient/common/src/linux/ais_conn.c \
	CameraMulticlient/common/src/ais_event_queue.c \
	CameraMulticlient/server/src/ais_server.c

LOCAL_C_INCLUDES:= \
	$(MY_AIS_ROOT)/API/inc \
	$(MY_AIS_ROOT)/CameraMulticlient/common/inc \
	$(MY_AIS_ROOT)/CameraMulticlient/server/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServices/inc \
	$(MY_AIS_ROOT)/CameraOSServices/CameraOSServicesMMOSAL/inc \
	$(MY_AIS_ROOT)/CameraQueue/CameraQueue/inc \
	$(MY_AIS_ROOT)/Common/inc \
	$(MY_AIS_ROOT)/Engine/inc

ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_HEADER_LIBRARIES := libmmosal_headers
ifneq (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_MODULE_PATH := $(TARGET_VENDOR_RAMDISK_OUT)/vendor_early_services/vendor/bin
else
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/earlyrootfs/system/bin
endif
else
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
endif


#Android R uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_headers
endif

LOCAL_CFLAGS := $(ais_compile_cflags) \
	-Wno-unused-parameter -Wno-sign-compare

ifeq ($(AIS_BUILD_STATIC),true)
LOCAL_WHOLE_STATIC_LIBRARIES  += libais
ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION))))
LOCAL_SHARED_LIBRARIES:= libmmosal_proprietary liblog libcutils libxml2
else
LOCAL_SHARED_LIBRARIES:= libmmosal liblog libcutils libxml2
endif
#Android R uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES := libmmosal liblog libcutils libxml2
endif
else
ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13  U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION))))
LOCAL_SHARED_LIBRARIES:= libais libais_log_proprietary liblog libcutils libbase libutils libdmabufheap
# AIS_BUILD_FOR_EARLYSERVICE will be true only if there ia an
# early partition on target, in other case we need to
# launch it from early kernel init
ifeq (($(TARGET_BOARD_PLATFORM),sdmshrike_au) || ($(AIS_BUILD_FOR_EARLYSERVICE),true))
LOCAL_INIT_RC :=
else
LOCAL_INIT_RC := ais_server_on_early_init.rc
endif
else
LOCAL_SHARED_LIBRARIES:= libais libais_log
endif
endif

ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
ifneq (,$(filter Tiramisu 13  U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
endif
endif

LOCAL_MODULE:= ais_server
LOCAL_MODULE_TAGS := optional

LOCAL_POST_INSTALL_CMD := \
      cp -rf $(TARGET_VENDOR_RAMDISK_OUT)/vendor_early_services/vendor/bin/$(LOCAL_MODULE) $(TARGET_OUT_VENDOR)/bin/

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)
