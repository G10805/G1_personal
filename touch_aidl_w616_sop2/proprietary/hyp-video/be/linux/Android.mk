LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#               Common definitons
# ---------------------------------------------------------------------------------

libhyp-video-be-def = -g -O3
libhyp-video-be-def += -Werror
libhyp-video-be-def += -D_ANDROID_

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
libhyp-video-be-def += -DSUPPORT_DMABUF
endif

# Common Includes
libhyp-video-be-inc := $(LOCAL_PATH)/inc
libhyp-video-be-inc += $(LOCAL_PATH)/../common/inc
libhyp-video-be-inc += $(LOCAL_PATH)/../../common/inc
libhyp-video-be-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
libhyp-video-be-inc += $(TARGET_OUT_HEADERS)/mm-hab
libhyp-video-be-inc += $(TOP)/vendor/qcom/proprietary/mm-osal/inc
libhyp-video-be-inc += $(TOP)/vendor/qcom/proprietary/common/inc

# Common Dependencies
libhyp-video-be-add-dep := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr


# ---------------------------------------------------------------------------------
#           Make the Shared library (libhyp-video-fe)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE                    := hyp_video_be
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(libhyp-video-be-def)
LOCAL_C_INCLUDES                += $(libhyp-video-be-inc)
LOCAL_ADDITIONAL_DEPENDENCIES   := $(libhyp-video-be-add-dep)

LOCAL_HEADER_LIBRARIES  := qti_video_kernel_uapi
LOCAL_SHARED_LIBRARIES  += liblog libcutils libmmosal

LOCAL_SRC_FILES         := ../../common/src/hyp_buffer_manager.cpp
LOCAL_SRC_FILES         += ../../common/src/hyp_queue_utility.cpp
LOCAL_SRC_FILES         += ../common/src/hyp_video_be.cpp
LOCAL_SRC_FILES         += ../common/src/hyp_video_be_translation.cpp
LOCAL_SRC_FILES         += src/linux_video_be.cpp

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)


# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------
