#======================================================================
# qcarcam_rvc
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS :=

LOCAL_SRC_FILES:= src/qcarcam_rvc.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../../API/inc \
        $(LOCAL_PATH)/../test_util/src/la \
	$(LOCAL_PATH)/../../Common/inc \
	$(LOCAL_PATH)/../../CameraOSServices/CameraOSServices/inc \
	$(LOCAL_PATH)/../test_util/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include \
	$(TARGET_OUT_HEADERS)/common/inc

LOCAL_CFLAGS :=-Werror \
	-DC2D_DISABLED \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers

LOCAL_SHARED_LIBRARIES:= libais_client liblog libutils

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15 V VanillaIceCream 15, $(PLATFORM_VERSION))))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
#Android R uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_headers
LOCAL_SHARED_LIBRARIES += libais_test_util_proprietary
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif
else
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_SHARED_LIBRARIES += libais_test_util_proprietary libmmosal_proprietary
endif
else
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/bin
LOCAL_HEADER_LIBRARIES := libmmosal_headers
LOCAL_SHARED_LIBRARIES += libais_test_util
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif
endif

LOCAL_MODULE:= qcarcam_rvc

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

#======================================================================
# qcarcam_edrm_rvc
#======================================================================
include $(CLEAR_VARS)

ifeq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
ifneq (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_LDFLAGS := -Wl,-rpath,'/vendor_early_services/system/lib64' -Wl,--dynamic-linker,/vendor_early_services/system/bin/bootstrap/linker64
LOCAL_CFLAGS += -DANDROID_T_ABOVE
endif
endif

LOCAL_SRC_FILES:= src/qcarcam_rvc.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../../API/inc \
	$(LOCAL_PATH)/../../Common/inc \
	$(LOCAL_PATH)/../../CameraOSServices/CameraOSServices/inc \
	$(LOCAL_PATH)/../test_util/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include \
	$(TARGET_OUT_HEADERS)/common/inc

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_CFLAGS += -Werror \
	-DC2D_DISABLED \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers

ifneq (,$(filter V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_CFLAGS += -DRVC_SELF_EXIT
endif

LOCAL_SHARED_LIBRARIES:= libais_client liblog libutils

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
#Android R and above uses libmmsoal, overwrite headers and libs
ifeq ( ,$(filter Q 10, $(PLATFORM_VERSION)))
LOCAL_HEADER_LIBRARIES := libmmosal_headers
LOCAL_SHARED_LIBRARIES += libais_test_util_edrm_proprietary
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
else
LOCAL_HEADER_LIBRARIES := libmmosal_proprietary_headers
LOCAL_SHARED_LIBRARIES += libais_test_util_edrm_proprietary libmmosal_proprietary
endif #PLATFORM_VERSION R_ABOVE
else
ifneq (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES += libais_test_util_edrm_proprietary
LOCAL_MODULE_PATH := $(TARGET_VENDOR_RAMDISK_OUT)/vendor_early_services/vendor/bin
else
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/earlyrootfs/system/bin
LOCAL_SHARED_LIBRARIES += libais_test_util_edrm
endif #PLATFORM_VERSION T_ABOVE
LOCAL_HEADER_LIBRARIES := libmmosal_headers
ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
LOCAL_SHARED_LIBRARIES += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES += libmmosal
endif #PLATFORM_VERSION U_ABOVE
endif #AIS_BUILD_FOR_EARLYSERVICE

LOCAL_MODULE:= qcarcam_edrm_rvc

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)
#===================
#rc files
#===================
# Push rc file to vendor so that auto launch will be disabled
# to enable auto launch move rc file to /vendor/etc/init
include $(CLEAR_VARS)
LOCAL_MODULE:= qcarcam_edrm_rvc.rc
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := qcarcam_edrm_rvc.rc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/
include $(BUILD_PREBUILT)
