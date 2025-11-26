LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

QC_PROP_ROOT ?= vendor/qcom/proprietary
SECUREMSM_SHIP_PATH := $(TOP)/$(QC_PROP_ROOT)/securemsm

LOCAL_HEADER_LIBRARIES := vendor_common_inc libbinder_ndk_headers libbinder_ndk_helper_headers
LOCAL_STATIC_LIBRARIES += libgtest

LOCAL_C_INCLUDES += $(SECUREMSM_SHIP_PATH)/mink/inc/interface \
                    $(SECUREMSM_SHIP_PATH)/smcinvoke/TZCom/inc

LOCAL_SHARED_LIBRARIES := libbinder_ndk \
                          libcutils \
                          liblog \
                          libutils \
                          vendor.qti.hardware.c2pa-V1-ndk

LOCAL_MODULE := C2PATestClientAIDL
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := C2PATestClientAIDL.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)
