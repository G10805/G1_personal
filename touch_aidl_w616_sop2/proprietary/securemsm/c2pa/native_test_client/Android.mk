LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

QC_PROP_ROOT ?= vendor/qcom/proprietary
SECUREMSM_SHIP_PATH := $(TOP)/$(QC_PROP_ROOT)/securemsm

LOCAL_HEADER_LIBRARIES := qtvm_sdk_headers vendor_common_inc libc2pa_tag_header
LOCAL_STATIC_LIBRARIES += libgtest

LOCAL_C_INCLUDES += $(SECUREMSM_SHIP_PATH)/mink/inc/interface \
                    $(SECUREMSM_SHIP_PATH)/smcinvoke/QCBOR/inc

LOCAL_SHARED_LIBRARIES := liblog \
                          libcutils \
                          libqcbor \
                          libutils \
                          libminksocket_vendor \
                          libdmabufheap \
                          libutils

LOCAL_MODULE := C2PANativeTestClient
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := C2PATestClient.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)
