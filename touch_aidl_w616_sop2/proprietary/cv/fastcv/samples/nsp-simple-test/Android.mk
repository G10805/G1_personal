LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#============================================================================
#Build fastcv_nsp_simple_test
#============================================================================

LOCAL_MODULE := fastcv_nsp_simple_test
#LOCAL_MODULE_TAGS := optional
LOCAL_CPP_EXTENSION := .cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/stub/inc
LOCAL_HEADER_LIBRARIES := libfastcv_vendor_headers
LOCAL_HEADER_LIBRARIES += libfastrpc_vendor_headers
LOCAL_SRC_FILES := src/fastcvNSPSimpleTest.cpp \
                   stub/src/fastcvNSPSimpleTest_stub.c
LOCAL_SHARED_LIBRARIES := libc libdl liblog libvndksupport libcdsprpc
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
    LOCAL_SRC_FILES    := skel/lib/libfastcvNSPSimpleTest_skel.so
    LOCAL_MODULE       := libfastcvNSPSimpleTest_skel.so
    LOCAL_MODULE_TAGS  := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib/rfsa/adsp
    LOCAL_MODULE_OWNER := qti
    LOCAL_PROPRIETARY_MODULE := true
    LOCAL_CHECK_ELF_FILES := false
include $(BUILD_PREBUILT)

