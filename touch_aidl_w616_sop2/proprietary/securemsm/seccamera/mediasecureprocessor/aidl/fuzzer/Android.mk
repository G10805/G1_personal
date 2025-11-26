LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := aidl_fuzzer_secure_processor

LOCAL_SRC_FILES := fuzzer.cpp \
    ../common/utils.cpp \
    ../common/SecureProcessor.cpp \
    ../tee-processor/SecureProcessorQTEEMink.cpp \
    ../tvm-processor/SecureProcessorTVMMink.cpp

LOCAL_C_INCLUDES := $(SECUREMSM_PATH)/smcinvoke/inc \
                    $(SECUREMSM_PATH)/smcinvoke/TZCom/inc \
                    $(SECUREMSM_PATH)/mink/inc/interface/ \
                    $(SECUREMSM_PATH)/mink/inc/qtvm/ \
                    $(SECUREMSM_PATH)/mink/inc/uid/ \
                    $(LOCAL_PATH)/../tee-processor \
                    $(LOCAL_PATH)/../tvm-processor \
                    $(LOCAL_PATH)/../common \
                    $(SECUREMSM_PATH)/smcinvoke/letzd/include

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libbase \
    libutils \
    libhardware \
    libbinder_ndk \
    libhidlbase \
    android.hardware.common-V2-ndk \
    vendor.qti.hardware.secureprocessor.common-helper \
    vendor.qti.hardware.secureprocessor.device-V1-ndk \
    vendor.qti.hardware.secureprocessor.common-V1-ndk \
    vendor.qti.hardware.secureprocessor.config-V1-ndk \
    libgralloc.qti \
    libgralloctypes \
    libdmabufheap \
    libminksocket_vendor \
    libminkdescriptor \
    libbinder \
    libclang_rt.ubsan_standalone

LOCAL_STATIC_LIBRARIES := libaidlcommonsupport

LOCAL_HEADER_LIBRARIES := \
    display_intf_headers \
    vendor_common_inc \
    qtvm_sdk_headers

LOCAL_CFLAGS := -g -O3 -Wno-unused-parameter -Wall -Werror -Woverloaded-virtual
LOCAL_CFLAGS += -D__ANDROID__

LOCAL_STATIC_LIBRARIES += libbinder_random_parcel
include $(BUILD_FUZZ_TEST)
