ROOT_DIR := $(call my-dir)

include $(CLEAR_VARS)
include $(LIBION_HEADER_PATH_WRAPPER)

LOCAL_PATH:= $(ROOT_DIR)

MSM_VIDC_GKI_TARGET_LIST := msmnile

ifneq ($(TARGET_KERNEL_VERSION), $(filter $(TARGET_KERNEL_VERSION), 4.14 4.19 4.9 5.4))
LOCAL_CFLAGS += -DSUPPORT_DMABUF
endif

ifneq ($(TARGET_KERNEL_VERSION), $(filter $(TARGET_KERNEL_VERSION), 4.14 4.19 4.9 5.4 5.15))
LOCAL_CFLAGS += -DANDROID_U_AND_ABOVE
endif

LOCAL_CFLAGS += -Wno-literal-suffix

LOCAL_MODULE                    := earlyVideo
LOCAL_PROPRIETARY_MODULE        := true
LOCAL_PRELINK_MODULE            := false
LOCAL_C_INCLUDES                += $(LIBION_HEADER_PATHS)
LOCAL_C_INCLUDES                += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES                += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_C_INCLUDES                += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/display
LOCAL_C_INCLUDES                += $(TOP)/external/libdrm
LOCAL_C_INCLUDES                += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/vidc
LOCAL_C_INCLUDES                += $(TARGET_OUT_HEADERS)/mm-video/utils
LOCAL_C_INCLUDES                += $(TOP)/vendor/qcom/proprietary/mm-video-utils/common-headers/utils
LOCAL_C_INCLUDES                += $(TOP)/vendor/qcom/proprietary/mm-video-utils/common-headers/streamparser
LOCAL_C_INCLUDES                += $(LOCAL_PATH)/common/inc
LOCAL_C_INCLUDES                += $(LOCAL_PATH)/utilities/inc
LOCAL_C_INCLUDES                += $(TOP)/system/core/include
LOCAL_ADDITIONAL_DEPENDENCIES   := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES  := libcutils libutils liblog libion libdrm

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
LOCAL_SHARED_LIBRARIES  += libdmabufheap
endif

LOCAL_HEADER_LIBRARIES := libmmvideo_common_headers qti_video_kernel_uapi
LOCAL_SRC_FILES        := common/src/VideoSession.cpp \
                          common/src/VideoSessionPort.cpp \
                          common/src/VideoDecoder.cpp \
                          common/src/VideoTestApp.cpp \
                          utilities/src/VideoMsgQueue.cpp \
                          utilities/src/VideoRender.cpp \
                          utilities/src/VideoH264StreamParser.cpp \
                          utilities/src/ll.cpp \

LOCAL_MODULE_TAGS      := optional

ifeq ($(BOARD_SUPPORTS_RAMDISK_EARLY_INIT), true)
ifneq ($(PLATFORM_VERSION), $(filter $(PLATFORM_VERSION),10 Q 11 R 12 S))
LOCAL_HEADER_LIBRARIES += qti_kernel_headers qti_display_kernel_headers device_kernel_headers
LOCAL_CFLAGS           += -D__ANDROID_T_AND_ABOVE__
LOCAL_MODULE_PATH      := $(TARGET_VENDOR_RAMDISK_OUT)/vendor_early_services/vendor/bin
LOCAL_LDFLAGS          := -Wl,-rpath,'/vendor_early_services/system/lib64' -Wl,--dynamic-linker,/vendor_early_services/system/bin/bootstrap/linker64
else
LOCAL_MODULE_PATH      := $(TARGET_VENDOR_RAMDISK_OUT)/early_services/vendor/bin
LOCAL_LDFLAGS          := -Wl,-rpath,'/early_services/system/lib64' -Wl,--dynamic-linker,/early_services/system/bin/bootstrap/linker64
endif
LOCAL_CFLAGS           += -D__EARLYSERVICES__
endif

LOCAL_CLANG            := true
include $(BUILD_EXECUTABLE)
