#======================================================================
# qcarcam_test - vendor app
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS :=

LOCAL_SRC_FILES:= src/qcarcam_test.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../../API/inc \
	$(LOCAL_PATH)/../../API/vendor \
	$(LOCAL_PATH)/../../Common/inc \
	$(LOCAL_PATH)/../../CameraOSServices/CameraOSServices/inc \
	$(LOCAL_PATH)/../test_util/inc \
	$(TARGET_OUT_HEADERS)/mm-osal/include \
	$(TARGET_OUT_HEADERS)/common/inc

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_CFLAGS :=-Werror \
	-DC2D_DISABLED \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers


LOCAL_SHARED_LIBRARIES:= libais_client liblog libutils

ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION))))
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
LOCAL_SHARED_LIBRARIES += libais_test_util libmmosal
endif

LOCAL_MODULE:= qcarcam_test

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)
