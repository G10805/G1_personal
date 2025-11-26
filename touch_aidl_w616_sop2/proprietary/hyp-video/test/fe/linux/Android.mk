LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS      := -D_ANDROID_
LOCAL_MODULE      := hypvideotest
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_C_INCLUDES  = $(LOCAL_PATH)/inc \
                    $(LOCAL_PATH)/../../../common/inc \
                    $(LOCAL_PATH)/../../../fe/common/inc \
                    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                    $(TARGET_OUT_HEADERS)/common/inc \
                    $(TARGET_OUT_HEADERS)/mm-osal/include

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES  := qti_video_kernel_uapi
LOCAL_SHARED_LIBRARIES  := liblog libcutils libhyp_video_fe

LOCAL_SRC_FILES   = src/hyp_video_test.cpp \
                    src/hyp_vinput_test.cpp

include $(BUILD_EXECUTABLE)

