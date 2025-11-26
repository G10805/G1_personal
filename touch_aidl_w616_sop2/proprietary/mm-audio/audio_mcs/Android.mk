LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE        := libmcs
LOCAL_MODULE_OWNER  := qti
LOCAL_MODULE_TAGS   := optional
LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES  := mcs/src/mcs.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/mcs/inc
LOCAL_C_INCLUDES += $(call include-path-for, audio-route)

LOCAL_HEADER_LIBRARIES := libspf-headers
LOCAL_HEADER_LIBRARIES += libacdb_headers

LOCAL_SHARED_LIBRARIES := \
    libagm \
    libaudioroute\
    libar-acdb\
    liblog\
    libcutils\
    libats\
    liblx-osal

#if android version is R, use qtitinyalsa lib otherwise use upstream ones
#This assumes we would be using AR code only for Android R and subsequent versions
ifneq ($(filter R 11,$(PLATFORM_VERSION)),)
LOCAL_SHARED_LIBRARIES += libqti-tinyalsa
else
LOCAL_SHARED_LIBRARIES += libtinyalsa
endif

include $(BUILD_SHARED_LIBRARY)

# Generate mcs_test.
include $(CLEAR_VARS)

LOCAL_MODULE        := mcs_test
LOCAL_MODULE_OWNER  := qti
LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES     := mcs/test/src/mcs-test.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/mcs/inc

LOCAL_HEADER_LIBRARIES := libspf-headers
LOCAL_HEADER_LIBRARIES += libacdb_headers

LOCAL_SHARED_LIBRARIES := \
    libaudioroute\
    liblog\
    libats\
    libar-acdb\
    liblx-osal\
    libmcs

#if android version is R, use qtitinyalsa lib otherwise use upstream ones
#This assumes we would be using AR code only for Android R and subsequent versions
ifneq ($(filter R 11,$(PLATFORM_VERSION)),)
LOCAL_SHARED_LIBRARIES += libqti-tinyalsa
else
LOCAL_SHARED_LIBRARIES += libtinyalsa
endif

include $(BUILD_EXECUTABLE)

