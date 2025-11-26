#============================================================================
#Build fastcv_simple_test
#============================================================================

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := fastcv_simple_test
#LOCAL_MODULE_TAGS := optional
LOCAL_CPP_EXTENSION := .cpp
LOCAL_C_INCLUDES :=  inc
LOCAL_HEADER_LIBRARIES := libfastcv_vendor_headers
LOCAL_SRC_FILES := fastcvSimpleTest.cpp
LOCAL_SHARED_LIBRARIES := libc libdl liblog libvndksupport libfastcvopt
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
