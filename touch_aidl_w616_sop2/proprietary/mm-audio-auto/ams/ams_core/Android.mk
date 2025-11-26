LOCAL_PATH := $(call my-dir)
# Build libamscore_headers
include $(CLEAR_VARS)
LOCAL_MODULE                := libamscore_headers
LOCAL_VENDOR_MODULE         := true
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE        := libamscore
LOCAL_MODULE_OWNER  := qti
LOCAL_MODULE_TAGS   := optional
LOCAL_VENDOR_MODULE := true

LOCAL_CFLAGS        := -D_ANDROID_ -DAMS_AGM_HW_RSC_CONFIG_V2_EN -DAMS_SSR_SUPPORT_EN -DAMS_CORE_RSC_FREE_SHMEM
LOCAL_CFLAGS        += -Wno-tautological-compare -Wno-macro-redefined -Wall
LOCAL_CFLAGS        += -DAMS_CORE_DSP_COMM_EN
LOCAL_C_INCLUDES    := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES    += $(LOCAL_PATH)/../ams_osal/inc

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION), 4.14 4.19 5.4))
LOCAL_CFLAGS        += -DAMS_USE_ION
else
LOCAL_CFLAGS        += -DAMS_USE_DMABUF
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/ar_osal
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/gpr
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-audio/ar/agm

include $(LIBION_HEADER_PATH_WRAPPER)
LOCAL_C_INCLUDES += $(LIBION_HEADER_PATHS)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc

LOCAL_SRC_FILES  := \
    src/ams_rsc_mgr.c \
    src/ams_prm.c \
    src/asic_8155/ams_endpoint_mgr.c \
    src/ams_gpr_common.c \
    src/ams_core.c \
    src/ams_core_cfg.c

LOCAL_HEADER_LIBRARIES := \
    libspf-headers \
    libutils_headers \
    libarosal_headers \
    libamsosal_headers

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libar-gpr-ams \
    liblx-osal \
    libion \
    libamsosal

ifneq ($(filter R 11,$(PLATFORM_VERSION)),)
LOCAL_SHARED_LIBRARIES += libqti-tinyalsa
LOCAL_C_INCLUDES       += $(TOP)/vendor/qcom/opensource/tinyalsa/include
else
LOCAL_SHARED_LIBRARIES += libtinyalsa
endif

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := ams_core.cfg
LOCAL_MODULE_FILENAME   := ams_core.cfg
LOCAL_MODULE_TAGS       := optional
LOCAL_MODULE_CLASS      := ETC
LOCAL_MODULE_PATH       := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES         := cfg/ams_core.cfg
include $(BUILD_PREBUILT)