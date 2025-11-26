LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#               Common definitons
# ---------------------------------------------------------------------------------

ENABLE_DVR_MUXER   =  0

hyp-dvr-mux-be-def = -g -O3
hyp-dvr-mux-be-def += -Werror
hyp-dvr-mux-be-def += -D_ANDROID_
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
hyp-dvr-mux-be-def += -DSUPPORT_DMABUF
endif

# Common Includes
hyp-dvr-mux-be-inc := $(LOCAL_PATH)/inc
hyp-dvr-mux-be-inc += $(LOCAL_PATH)/../common/inc
hyp-dvr-mux-be-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
hyp-dvr-mux-be-inc += $(TARGET_OUT_HEADERS)/mm-hab
hyp-dvr-mux-be-inc += $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-osal/inc
hyp-dvr-mux-be-inc += $(TOP)/vendor/qcom/proprietary/common/inc
hyp-dvr-mux-be-inc += $(TOP)/frameworks/av/media/libmedia/include
hyp-dvr-mux-be-inc += $(TOP)/frameworks/av/media/libstagefright/include/media/stagefright
hyp-dvr-mux-be-inc += $(TOP)/frameworks/av/include
hyp-dvr-mux-be-inc += $(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc
hyp-dvr-mux-be-inc += $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-mux/main/FileMuxLib/inc
hyp-dvr-mux-be-inc += $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-mux/main/FilemuxInternalDefs/inc
hyp-dvr-mux-be-inc += $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-mux/main/MP2BaseFileLib/inc
hyp-dvr-mux-be-inc += $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-mux/main/MuxBaseLib/inc
hyp-dvr-mux-be-inc += $(TOP)/frameworks/av/media/libmediametrics/include
hyp-dvr-mux-be-inc += $(TOP)/frameworks/av/media/libstagefright/foundation/include/media/stagefright
hyp-dvr-mux-be-inc += $(TOP)/kernel/msm-5.4/techpack/video/include/uapi/vidc

# Common Dependencies
hyp-dvr-mux-be-add-dep := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr


# ---------------------------------------------------------------------------------
#           Make the executable (hyp_dvr_mux_be)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE                    := hyp_dvr_mux_be
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(hyp-dvr-mux-be-def)
LOCAL_C_INCLUDES                += $(hyp-dvr-mux-be-inc)
LOCAL_ADDITIONAL_DEPENDENCIES   := $(hyp-dvr-mux-be-add-dep)

LOCAL_HEADER_LIBRARIES  := qti_video_kernel_uapi
ifeq ($(ENABLE_DVR_MUXER),1)
LOCAL_SHARED_LIBRARIES  += liblog libcutils libmmosal libstagefright_foundation libbinder libutils libgui_vendor libui \
                           libFileMux_proprietary
else
LOCAL_SHARED_LIBRARIES  += liblog libcutils libmmosal libstagefright_foundation libbinder libutils
endif

LOCAL_SRC_FILES         := src/hyp_dvr_mux_be.cpp
LOCAL_SRC_FILES         += ../common/src/hyp_buffer_manager.cpp

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)


# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------
