LOCAL_PATH := $(call my-dir)
QCRIL_NR_DIR := ${LOCAL_PATH}/../..

include $(CLEAR_VARS)

LOCAL_CFLAGS               += $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++17 $(qcril_cppflags)
LOCAL_CPPFLAGS             += -std=c++17 $(qcril_cppflags)
LOCAL_LDFLAGS              += $(qcril_ldflags)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE             += $(qcril_sanitize)
LOCAL_NOSANITIZE           := cfi
endif
LOCAL_SRC_FILES            := src/QtiRadio.cpp
LOCAL_SRC_FILES            += src/QtiRadioStableAidl.cpp
LOCAL_SRC_FILES            += src/stable_aidl_impl/qti_radio_stable_aidl_service.cpp
LOCAL_SRC_FILES            += src/stable_aidl_impl/qti_radio_stable_aidl_service_utils.cpp
LOCAL_SRC_FILES            += src/hidl_impl/qti_radio_service_base.cpp
LOCAL_SRC_FILES            += src/hidl_impl/1.0/qti_radio_service_1_0.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.0/qti_radio_service_2_0.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.0/qti_radio_service_utils_2_0.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.1/qti_radio_service_2_1.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.1/qti_radio_service_utils_2_1.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.2/qti_radio_service_2_2.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.2/qti_radio_service_utils_2_2.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.3/qti_radio_service_2_3.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.4/qti_radio_service_2_4.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.5/qti_radio_service_2_5.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.5/qti_radio_service_utils_2_5.cpp
LOCAL_SRC_FILES            += src/hidl_impl/2.6/qti_radio_service_2_6.cpp

LOCAL_MODULE               := qcrilNrQtiRadio
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_HEADER_LIBRARIES     += libqcrilNr-headers \
                              libqcrilNrQtiMutex-headers \
                              qtiril-utils-headers
LOCAL_HEADER_LIBRARIES     += libqcrilNrLogger-headers
LOCAL_HEADER_LIBRARIES     += qtiRilHalUtil-headers
LOCAL_HEADER_LIBRARIES     += libril-db-headers
LOCAL_HEADER_LIBRARIES     += libril-legacy-headers
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@1.0
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@2.0
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@2.1
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@2.2
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@2.3
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@2.4
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@2.5
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@2.6
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio-V8-ndk_platform
LOCAL_SHARED_LIBRARIES     += libbinder_ndk
LOCAL_SHARED_LIBRARIES     += libbase

LOCAL_SHARED_LIBRARIES     += libqcrilNrQtiMutex
LOCAL_SHARED_LIBRARIES     += qtiril-utils
LOCAL_SHARED_LIBRARIES     += libril-db
LOCAL_SHARED_LIBRARIES     += libril-legacy
LOCAL_C_INCLUDES           += $(LOCAL_PATH)/src/
LOCAL_LDLIBS               := -llog
include $(BUILD_STATIC_LIBRARY)
