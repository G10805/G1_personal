ifneq ($(ENABLE_HYP),true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin

LOCAL_C_INCLUDES:= $(TARGET_OUT_HEADERS)/mm-audio/audio-acdb-util/ \
                    vendor/qcom/opensource/audio-hal/primary-hal/hal/ \
                    hardware/libhardware/include \
                    system/media/audio/include \
                    external/tinycompress/include \
                    $(call include-path-for, audio-route) \
                    system/media/audio_utils/include \
                    vendor/qcom/opensource/audio-hal/primary-hal/hal/audio_extn/

LOCAL_HEADER_LIBRARIES += libhardware_headers
LOCAL_HEADER_LIBRARIES += libcutils_headers
LOCAL_SRC_FILES:= audio_chime_app.c
LOCAL_MODULE := audio_chime
LOCAL_SHARED_LIBRARIES:= libcutils libutils libtinyalsa libdl liblog
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

endif # ENABLE_HYP

