LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../common/inc \
    $(TOP)/frameworks/native/include \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
    $(TARGET_OUT_HEADERS)/mm-hab \
    $(TOP)/frameworks/native/libs/nativebase/include \
    $(TOP)/system/libhidl/transport/token/1.0/utils/include \
    $(TOP)/vendor/qcom/proprietary/commonsys-intf/mm-osal/inc \
    $(TOP)/vendor/qcom/proprietary/common/inc \
    $(TOP)/kernel/msm-5.4/techpack/video/include/uapi/vidc \
    $(TOP)/kernel/msm-5.4/include/uapi

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_CFLAGS := -D_ANDROID_
LOCAL_MODULE := vendor.qti.dvr@1.0-service
LOCAL_INIT_RC := vendor.qti.dvr@1.0-service.rc
LOCAL_MODULE_TAGS  := optional
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_SRC_FILES := \
    service.cpp hyp_dvr_display_be.cpp \
    ../common/src/hyp_buffer_manager.cpp

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),4.14 4.19 4.9 5.4))
LOCAL_CFLAGS += -DSUPPORT_DMABUF
endif

LOCAL_HEADER_LIBRARIES := qti_video_kernel_uapi
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    libmmosal \
    vendor.qti.dvr@1.0 \

include $(BUILD_EXECUTABLE)
