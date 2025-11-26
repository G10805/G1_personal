LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    src/audcalparam_api.c \
    src/audcalparam_serv_con.c

LOCAL_EXPORT_C_INCLUDE_DIRS   := $(LOCAL_PATH)/inc/

libaudcalparam-def := -g -O3
libaudcalparam-def += -D_ANDROID_
libaudcalparam-def += -D AUDIO_USE_SYSTEM_HEAP_ID
libaudcalparam-def += -D USE_LIBACDBLOADER
#libaudcalparam-def += -D AUDCALPARAM_DBG
#libaudcalparam-def += -D AUDCALPARAM_BUF_DBG
LOCAL_CFLAGS       = $(libaudcalparam-def)

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdl \
    libjson

LOCAL_C_INCLUDES        = $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc/
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/libjson/inc/
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/audio
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_TAGS := optional

ifeq ($(ENABLE_AUDIO_LEGACY_TECHPACK),true)
LOCAL_HEADER_LIBRARIES += qti_legacy_audio_kernel_uapi
endif

LOCAL_HEADER_LIBRARIES += vendor_common_inc
LOCAL_HEADER_LIBRARIES += libacdbloader_headers

LOCAL_MODULE:= libaudcalparam
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

