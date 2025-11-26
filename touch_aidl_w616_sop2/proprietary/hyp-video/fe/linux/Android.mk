LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#               Common definitons
# ---------------------------------------------------------------------------------

libhyp-video-fe-def = -g -O3
libhyp-video-fe-def += -Werror
libhyp-video-fe-def += -D_ANDROID_
libhyp-video-fe-def += -D_ANDROID_ICS_
libhyp-video-fe-def += -D_C2_HAL_
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
libhyp-video-fe-def += -DSUPPORT_DMABUF
endif

# Common Includes
libhyp-video-fe-inc          := $(LOCAL_PATH)/inc
libhyp-video-fe-inc          += $(LOCAL_PATH)/../common/inc
libhyp-video-fe-inc          += $(LOCAL_PATH)/../../common/inc
libhyp-video-fe-inc          += $(TOP)/system/core/libutils/include
libhyp-video-fe-inc          += $(TOP)/system/memory/libion/include
libhyp-video-fe-inc          += $(TOP)/system/memory/libion/kernel-headers
libhyp-video-fe-inc          += $(TOP)/system/core/include

libhyp-video-fe-inc          += $(TOP)/hardware/qcom/media/mm-video-v4l2/vidc/common/inc
libhyp-video-fe-inc          += $(TOP)/hardware/qcom/media/mm-video-v4l2/vidc/vdec/inc
libhyp-video-fe-inc          += $(TOP)/hardware/qcom/media/mm-video-v4l2/vidc/venc/inc
libhyp-video-fe-inc          += $(TOP)/hardware/qcom/media/mm-core/inc
libhyp-video-fe-inc          += $(TOP)/hardware/qcom/media/mm-core/src/common
libhyp-video-fe-inc          += $(TOP)/hardware/qcom/media/libc2dcolorconvert
libhyp-video-fe-inc          += $(TOP)/hardware/libhardware/include
libhyp-video-fe-inc          += $(TOP)/hardware/qcom/display/libcopybit
libhyp-video-fe-inc          += $(TOP)/hardware/qcom/media/libarbitrarybytes/inc


libhyp-video-fe-inc          += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
libhyp-video-fe-inc          += $(TARGET_OUT_HEADERS)/mm-hab

libhyp-video-fe-inc          += $(TOP)/kernel/msm-5.4/techpack/video/include/uapi/vidc

libhyp-video-fe-inc += $(TOP)/hardware/qcom/media/libstagefrighthw
libhyp-video-fe-inc += $(TOP)/vendor/qcom/proprietary/mm-osal/inc
libhyp-video-fe-inc += $(TOP)/vendor/qcom/proprietary/common/inc
libhyp-video-fe-inc += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/libqdmetadata
libhyp-video-fe-inc += $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-osal/inc
libhyp-video-fe-inc += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/include
libhyp-video-fe-inc += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc


# Common Dependencies
libhyp-video-fe-add-dep := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr


# ---------------------------------------------------------------------------------
#           Make the Shared library (libhyp-video-fe)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE                    := libhyp_video_fe
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(libhyp-video-fe-def)
LOCAL_C_INCLUDES                += $(libhyp-video-fe-inc)
LOCAL_ADDITIONAL_DEPENDENCIES   := $(libhyp-video-fe-add-dep)

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  += liblog libcutils libion
ifneq ($(PLATFORM_VERSION), $(filter $(PLATFORM_VERSION),10 Q 11 R 12 S 13 T))
LOCAL_SHARED_LIBRARIES  += libmmosal_vendor
else
LOCAL_SHARED_LIBRARIES  += libmmosal
endif
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
LOCAL_SHARED_LIBRARIES  += libdmabufheap
endif

LOCAL_HEADER_LIBRARIES := \
        media_plugin_headers \
        libnativebase_headers \
        libutils_headers \
        libhardware_headers \
        qti_video_kernel_uapi \
        qti_gfx_kernel_uapi \
        libmm-hab_headers

LOCAL_SRC_FILES         := ../../common/src/hyp_buffer_manager.cpp
LOCAL_SRC_FILES         += ../../common/src/hyp_queue_utility.cpp
LOCAL_SRC_FILES         += ../common/src/hyp_video_fe.cpp
LOCAL_SRC_FILES         += ../common/src/hyp_video_fe_translation.cpp
LOCAL_SRC_FILES         += ../common/src/hyp_video_meta_translator.cpp
LOCAL_SRC_FILES         += src/linux_video_fe.cpp
LOCAL_SRC_FILES         += src/linux_vdec_fe.cpp
LOCAL_SRC_FILES         += src/linux_vdec_venc_common.cpp
LOCAL_SRC_FILES         += src/linux_venc_fe.cpp
LOCAL_SRC_FILES         += src/linux_vinput_fe.cpp

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------
