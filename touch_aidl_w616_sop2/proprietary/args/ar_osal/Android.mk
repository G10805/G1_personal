LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libarosal_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/api
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

#----------------------------------------------------------------------------
#                 Common definitons
#----------------------------------------------------------------------------

osal-def += -D_ANDROID_

#----------------------------------------------------------------------------
#             Make the Shared library (liblx-osal)
#----------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/api

LOCAL_CFLAGS := $(osal-def)
LOCAL_HEADER_LIBRARIES += audio_kernel_headers
LOCAL_HEADER_LIBRARIES += libarosal_headers
LOCAL_HEADER_LIBRARIES += qti_audio_kernel_uapi
LOCAL_HEADER_LIBRARIES += libdiag_headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS := libarosal_headers

ifeq ($(TARGET_USES_QSSI), true)
    include $(LIBION_HEADER_PATH_WRAPPER)
    LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
else ifeq ($(TARGET_GVMGH_SPECIFIC), false)
    include $(LIBION_HEADER_PATH_WRAPPER)
    LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
else
    LOCAL_C_INCLUDES += $(TOP)/system/core/libion/include
    LOCAL_C_INCLUDES += $(TOP)/system/core/libion/kernel-headers
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES := src/ar_osal_mutex.c \
                   src/ar_osal_thread.c \
                   src/ar_osal_signal.c \
                   src/ar_osal_log.c \
                   src/ar_osal_log_pkt_op.c \
                   src/ar_osal_file_io.c \
                   src/ar_osal_sleep.c\
                   src/ar_osal_servreg.c\
                   src/ar_osal_mem_op.c\
                   src/ar_osal_heap.c\
                   src/ar_osal_timer.c\
                   src/ar_osal_string.c

ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION), 4.14 4.19 5.4))
LOCAL_SRC_FILES += src/ar_osal_shmem.c
else
LOCAL_SRC_FILES += src/ar_osal_shmem_db.c
endif

ifeq ($(strip $(TARGET_USES_ION_CMA_MEMORY)), true)
LOCAL_CFLAGS += -DTARGET_USES_ION_CMA_MEMORY
endif

LOCAL_SHARED_LIBRARIES := liblog \
                          libdiag

LOCAL_MODULE := liblx-osal
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

ifneq ($(strip $(AUDIO_FEATURE_OLD_ION_IMPL)), true)
        include $(LIBION_HEADER_PATH_WRAPPER)
        LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
        LOCAL_SHARED_LIBRARIES += libion
endif

LOCAL_HEADER_LIBRARIES += mm-audio/ar/ar_osal
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/api

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

include $(CLEAR_VARS)
LOCAL_SRC_FILES := test/src/ar_osal_test.c \
                   test/src/ar_test_file_io.c \
                   test/src/ar_test_shmem.c \
                   test/src/ar_test_signal_mutex_thread.c
LOCAL_MODULE := osal_in_test_example
LOCAL_MODULE_OWNER := qti

LOCAL_SHARED_LIBRARIES := liblog \
                          liblx-osal \
                          libdiag

LOCAL_C_INCLUDES := $(LOCAL_PATH)/api \
                    $(LOCAL_PATH)/test/inc

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include

LOCAL_HEADER_LIBRARIES := libdiag_headers
LOCAL_VENDOR_MODULE := true
include $(BUILD_EXECUTABLE)
