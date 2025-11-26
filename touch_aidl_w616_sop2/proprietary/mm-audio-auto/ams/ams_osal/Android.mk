LOCAL_PATH := $(call my-dir)
# Build libamscore_headers
include $(CLEAR_VARS)
LOCAL_MODULE                := libamsosal_headers
LOCAL_VENDOR_MODULE         := true
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE        := libamsosal
LOCAL_MODULE_OWNER  := qti
LOCAL_MODULE_TAGS   := optional
LOCAL_VENDOR_MODULE := true

LOCAL_CFLAGS        := -D_ANDROID_
LOCAL_CFLAGS        += -Wno-tautological-compare -Wno-macro-redefined -Wall

LOCAL_HEADER_LIBRARIES += libarosal_headers
LOCAL_HEADER_LIBRARIES += qti_audio_kernel_uapi
LOCAL_C_INCLUDES    := $(LOCAL_PATH)/inc

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/ar_osal

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

ifeq ($(TARGET_USES_QSSI), true)
    include $(LIBION_HEADER_PATH_WRAPPER)
    LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
else ifneq (,$(filter $(PRODUCT_NAME), msmnile_gvmq msmnile_au))
    include $(LIBION_HEADER_PATH_WRAPPER)
    LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
else
    LOCAL_C_INCLUDES += $(TOP)/system/core/libion/include
    LOCAL_C_INCLUDES += $(TOP)/system/core/libion/kernel-headers
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include


LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc

LOCAL_SRC_FILES  := \
    src/ams_osal_mutex.c \
    src/ams_osal_servreg.c \
    src/ams_osal_ssr.c


ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION), 4.14 4.19 5.4))
LOCAL_SRC_FILES += src/ams_osal_shmem.c
else
LOCAL_SRC_FILES += src/ams_osal_shmem_db.c
LOCAL_CFLAGS        += -DAMS_USE_DMABUF
endif

ifeq ($(strip $(TARGET_USES_ION_CMA_MEMORY)), true)
LOCAL_CFLAGS += -DTARGET_USES_ION_CMA_MEMORY
endif

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libar-gpr \
    liblx-osal \
    libion

ifneq ($(strip $(AUDIO_FEATURE_OLD_ION_IMPL)), true)
       include $(LIBION_HEADER_PATH_WRAPPER)
       LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
       LOCAL_SHARED_LIBRARIES += libion
endif

ifeq ($(TARGET_PD_SERVICE_ENABLED), true)
    LOCAL_SHARED_LIBRARIES += libpdmapper

    LOCAL_SHARED_LIBRARIES += libpdnotifier
    LOCAL_HEADER_LIBRARIES += libpdmapper_headers
    LOCAL_HEADER_LIBRARIES += libpdnotifier_headers
    LOCAL_HEADER_LIBRARIES += libqmi_legacy_headers
    LOCAL_HEADER_LIBRARIES += libqmi_common_headers
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libpdnotifier/inc
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
    LOCAL_CFLAGS += -DAR_OSAL_USE_PD_NOTIFIER
endif

LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_HEADER_LIBRARIES += vendor_common_inc

include $(BUILD_SHARED_LIBRARY)