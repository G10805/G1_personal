# verify it isn't a simulator build
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8998 apq8098_latv sdm845),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := spdaemon.c


LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../securemsm/spcomlib/inc


LOCAL_SHARED_LIBRARIES := libspcom libutils libcutils liblog libperipheral_client libmdmdetect

LOCAL_HEADER_LIBRARIES := libmdmdetect_headers vendor_common_inc \
			libmdmdetect_headers libperipheralclient_headers


LOCAL_CFLAGS += -fno-exceptions -fno-short-enums -DANDROID

ifeq ($(TARGET_BUILD_VARIANT),user)
LOCAL_CFLAGS+= -DUSER_BUILD
endif

LOCAL_MODULE := spdaemon

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qti

ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
LOCAL_PROPRIETARY_MODULE := true
endif

include $(BUILD_EXECUTABLE)

endif
endif

