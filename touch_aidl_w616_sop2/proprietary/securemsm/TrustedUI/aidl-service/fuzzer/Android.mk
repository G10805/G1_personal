LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

QC_PROP_ROOT ?= vendor/qcom/proprietary
SECUREMSM_SHIP_PATH := $(TOP)/$(QC_PROP_ROOT)/securemsm

LOCAL_MODULE := aidl_fuzzer_trustedui

LOCAL_SRC_FILES := fuzzer.cpp

LOCAL_C_INCLUDES := $(SECUREMSM_SHIP_PATH)/mink/inc/interface \
                    $(LOCAL_PATH)/../

LOCAL_HEADER_LIBRARIES := libhardware_headers libbinder_ndk_headers

LOCAL_SHARED_LIBRARIES := liblog \
                          libbase \
                          libutils \
                          libhardware \
                          libbinder_ndk \
                          vendor.qti.hardware.trustedui-V1-ndk \
                          libTrustedUIAIDL \
                          libTrustedInputAIDL \
                          libbinder \
                          libcutils \
                          libclang_rt.ubsan_standalone

LOCAL_STATIC_LIBRARIES += libbinder_random_parcel

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_FUZZ_TEST)