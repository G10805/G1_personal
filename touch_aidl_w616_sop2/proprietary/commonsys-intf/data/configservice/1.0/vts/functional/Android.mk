LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE        := VtsImsConfigServiceTest_1_0
LOCAL_MODULE_TAGS   := optional
LOCAL_CFLAGS        := -O0 -g


LOCAL_SRC_FILES     += ImsConfigServiceGenericTest.cpp
LOCAL_SRC_FILES     += ImsConfigAppTokenTest.cpp
LOCAL_SRC_FILES     += ImsConfigCarrierConfigTest.cpp

LOCAL_SHARED_LIBRARIES := liblog \
                          libhidlbase \
                          libhidltransport \
                          libutils \
                          libz \
                          vendor.qti.ims.factory@2.0 \
                          vendor.qti.ims.configservice@1.0 \


LOCAL_STATIC_LIBRARIES := VtsHalHidlTargetTestBase

LOCAL_MODULE_OWNER := qti

include $(BUILD_NATIVE_TEST)