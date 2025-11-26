ifeq ($(call math_gt, $(PLATFORM_SDK_VERSION), 29), false)

LOCAL_PATH:= $(call my-dir)
# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

########################################################
#
#  BUILD_EARLY_VIDEO=0, ETHERNET RVC is enabled(default)
#  BUILD_EARLY_VIDEO=1, EARLY_VIDEO is enabled
#
########################################################
BUILD_EARLY_VIDEO = 0
USE_SURFACE_FLINGER = 1

ifeq ($(USE_SURFACE_FLINGER),1)
libmm-vdec-def += -DNATIVEWINDOW_DISPLAY
endif

ifeq ($(BUILD_EARLY_VIDEO),1)
libmm-vdec-def += -DEARLY_BOOTVIDEO
else
ifeq ($(TARGET_ENABLE_QC_AV_ENHANCEMENTS),1)
libmm-vdec-def += -DREAD_FROM_SOCKET
endif
endif

libmm-vdec-def += -g -O3
libmm-vdec-def += -D_ANDROID_
libmm-vdec-def += -DFEATURE_FILESOURCE_FILE_PLAYBACK
libmm-vdec-def += -DUSE_ION
libmm-vdec-def += -D_MSM8974_

ifeq ($(call is-board-platform-in-list, $(TARGETS_THAT_HAVE_VENUS_HEVC)),true)
libmm-vdec-def += -DVENUS_HEVC
endif

ifeq ($(call is-board-platform-in-list, $(TARGETS_THAT_SUPPORT_UBWC)),true)
libmm-vdec-def += -D_UBWC_
endif


ifeq ($(TARGET_BUILD_VARIANT),$(filter $(TARGET_BUILD_VARIANT),userdebug eng))
libmm-vdec-def += "-DEDRM_RETRY_CNTS=300"
else
libmm-vdec-def += "-DEDRM_RETRY_CNTS=20"
endif

include $(CLEAR_VARS)

# Common Includes

LOCAL_C_INCLUDES:= \
        $(TOP)/system/core/libion/include \
	$(TOP)/system/core/libion/kernel-headers \
        $(TARGET_OUT_HEADERS)/mm-core/omxcore \
        $(TARGET_OUT_HEADERS)/qcom/display \
        $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
        $(TARGET_OUT_HEADERS)/mm-osal/include \
        $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-osal/inc/ \
        $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-parser-noship/FileBaseLib/inc/ \
        $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-rtp/decoder/inc/ \
        $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-rtp/encoder/inc/ \
        $(TARGET_OUT_HEADERS)/mm-parser/include \
        $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-parser/Android_adaptation/inc/ \
        $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-parser/Api/inc \
        $(TOP)/vendor/qcom/proprietary/mm-video-auto/mm-rtp-test/file-source-wapper \
        $(TOP)/vendor/qcom/proprietary/common/inc \
        $(TOP)/hardware/qcom/media/mm-core/inc \
        $(TOP)/external/libdrm \
        $(TOP)/external/mesa3d/src/gbm/main/ \
        $(TOP)/frameworks/native/include/media/hardware \
        $(TOP)/frameworks/native/include/media/openmax \
        $(TOP)/frameworks/av/media/libstagefright \
        $(TOP)/frameworks/av/media/libstagefright/include \
	$(TOP)/vendor/qcom/opensource/commonsys-intf/display/gralloc/ \
	$(TOP)/frameworks/native/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:=         \
        omx_vdec_test.cpp \
        omx_vdec_render_android.cpp \
        queue.c

LOCAL_SHARED_LIBRARIES := \
        libmmosal_proprietary liblog libutils libcutils libbinder libdrm libstagefright_foundation libmmparser_lite_proprietary libOmxCore

ifeq ($(TARGET_ENABLE_QC_AV_ENHANCEMENTS),1)
LOCAL_SHARED_LIBRARIES += libmmrtpdecoder_proprietary
endif

ifeq ($(USE_SURFACE_FLINGER),1)
LOCAL_SHARED_LIBRARIES += libstagefright_omx libgui_vendor libui libnativewindow
endif

LOCAL_CFLAGS                    := $(libmm-vdec-def)

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif


LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= videoearly

LOCAL_MODULE_OWNER               := qti
LOCAL_PROPRIETARY_MODULE         := true

include $(BUILD_EXECUTABLE)

endif
