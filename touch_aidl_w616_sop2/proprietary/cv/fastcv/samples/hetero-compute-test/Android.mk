LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#============================================================================
#Build fastcv_hetero_compute_test
#============================================================================

LOCAL_MODULE := fastcv_hetero_compute_test
#LOCAL_MODULE_TAGS := optional
LOCAL_CPP_EXTENSION := .cpp
LOCAL_HEADER_LIBRARIES := libfastcv_vendor_headers
LOCAL_SRC_FILES := src/fastcvHeteroComputeTest.cpp
LOCAL_SHARED_LIBRARIES := libc libdl liblog libvndksupport libfastcvopt
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
