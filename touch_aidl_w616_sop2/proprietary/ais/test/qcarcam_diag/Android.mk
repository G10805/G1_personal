#======================================================================
# qcarcam_diag - vendor app
#======================================================================
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS :=

LOCAL_SRC_FILES:= src/qcarcam_diag.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../../API/inc \
	$(LOCAL_PATH)/../../Common/inc \
	$(TARGET_OUT_HEADERS)/common/inc

LOCAL_CFLAGS :=-Werror \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers


LOCAL_SHARED_LIBRARIES:= libais_client liblog libutils

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

LOCAL_MODULE:= qcarcam_diag

LOCAL_PRELINK_MODULE:= false

ifeq ($(AIS_32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)
