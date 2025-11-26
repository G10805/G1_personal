LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libar-gpr
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := \
    core/src/gpr_drv.c \
    core/src/gpr_list.c \
    core/src/gpr_main.c \
    core/src/gpr_memq.c \
    core/src/gpr_drv_island.c \
    core/src/gpr_list_island.c \
    core/src/gpr_memq_island.c \
    core/src/hash_based/gpr_session.c \
    core/src/hash_based/gpr_session_island.c \
    ext/dynamic_allocation/src/gpr_dynamic_allocation.c \
    ext/src/gpr_log.c \
    datalinks/gpr_lx/src/gpr_lx.c \
    platform_wrappers/lx/gpr_init_lx_wrapper.c

LOCAL_SHARED_LIBRARIES := \
    liblx-osal\
    libcutils\
    liblog

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/api \
    $(LOCAL_PATH)/api/private \
    $(LOCAL_PATH)/core/inc \
    $(LOCAL_PATH)/core/src \
    $(LOCAL_PATH)/ext/dynamic_allocation/inc \
    $(LOCAL_PATH)/ext/inc \
    $(LOCAL_PATH)/datalinks/gpr_lx/inc

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/api
LOCAL_CFLAGS := -D_ANDROID_ \
        -DSESSION_ARRAY_SIZE=200


include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libar-gpr-ams
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SRC_FILES := \
    core/src/gpr_drv.c \
    core/src/gpr_list.c \
    core/src/gpr_main.c \
    core/src/gpr_memq.c \
    core/src/gpr_drv_island.c \
    core/src/gpr_list_island.c \
    core/src/gpr_memq_island.c \
    core/src/hash_based/gpr_session.c \
    core/src/hash_based/gpr_session_island.c \
    ext/dynamic_allocation/src/gpr_dynamic_allocation.c \
    ext/src/gpr_log.c \
    datalinks/gpr_lx/src/gpr_lx.c \
    platform_wrappers/lx/gpr_init_lx_wrapper.c

LOCAL_SHARED_LIBRARIES := \
    liblx-osal\
    libcutils\
    liblog

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/api \
    $(LOCAL_PATH)/api/private \
    $(LOCAL_PATH)/core/inc \
    $(LOCAL_PATH)/core/src \
    $(LOCAL_PATH)/ext/dynamic_allocation/inc \
    $(LOCAL_PATH)/ext/inc \
    $(LOCAL_PATH)/datalinks/gpr_lx/inc

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/api
LOCAL_CFLAGS := -D_ANDROID_ \
        -DSESSION_ARRAY_SIZE=200 \
        -DAMS_GPR_EN

ifeq ($(TARGET_SUPPORTS_WEAR_AON),true)
LOCAL_CFLAGS += -DPLATFORM_SLATE
endif

include $(BUILD_SHARED_LIBRARY)
