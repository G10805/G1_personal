ifeq ($(MM_AUDIO_ENABLED_FTM),true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

mm-audio-ftm-def += -g -O2
mm-audio-ftm-def += -DQC_MODIFIED
mm-audio-ftm-def += -D_ANDROID_
mm-audio-ftm-def += -DQCT_CFLAGS
mm-audio-ftm-def += -DQCT_CPPFLAGS
mm-audio-ftm-def += -DVERBOSE
mm-audio-ftm-def += -D_DEBUG
mm-audio-ftm-def += -DMSM8960_ALSA
mm-audio-ftm-def += -DVNDK_ENABLED

include $(CLEAR_VARS)

mm-audio-ftm-inc := $(LOCAL_PATH)/inc
mm-audio-ftm-inc += $(TARGET_OUT_HEADERS)/diag/include

LOCAL_MODULE             := mm-audio-ftm
LOCAL_MODULE_TAGS        := optional
LOCAL_MODULE_OWNER       := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CFLAGS             := $(mm-audio-ftm-def)
LOCAL_C_INCLUDES         := $(mm-audio-ftm-inc)

LOCAL_SRC_FILES := \
    src/DALSYS_common.c \
    src/audio_ftm_afe_loopback.c \
    src/audio_ftm_driver_fwk.c \
    src/audio_ftm_dtmf_basic_op.c \
    src/audio_ftm_dtmf_tone_gen.c \
    src/audio_ftm_hw_drv_ar.c \
    src/audio_ftm_pcm_loopback.c \
    src/audio_ftm_pcm_record.c \
    src/audio_ftm_tone_play.c \
    src/audio_ftm_util_fifo.c \
    src/ftm_audio_dispatch_ar.c \
    src/ftm_audio_main.c \
    src/audio_ftm_diag_mem.c \
    src/audio_ftm_pcm_play.c \
    src/audio_ftm_ext_loopback.c \
    src/fft.c

LOCAL_HEADER_LIBRARIES := \
    libacdb_headers \
    libagm_headers \
    libdiag_headers \
    libutils_headers \
    vendor_common_inc

LOCAL_SHARED_LIBRARIES  := libdiag libcutils libdl liblog

#only if android version is R, use headers from qtitinyalsa
#This assumes we would be using AR code only for Android R and subsequent versions
ifneq ($(filter R 11,$(PLATFORM_VERSION)),)
LOCAL_SHARED_LIBRARIES += libqti-tinyalsa
LOCAL_C_INCLUDES       += $(TOP)/vendor/qcom/opensource/tinyalsa/include
else
LOCAL_SHARED_LIBRARIES += libtinyalsa
endif

include $(BUILD_EXECUTABLE)

ifeq ($(call is-board-platform-in-list,lahaina),true)

include $(CLEAR_VARS)
LOCAL_MODULE       := ftm_test_config
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES    := config/lahaina/ftm_test_config
include $(BUILD_PREBUILT)
endif

ifeq ($(call is-board-platform-in-list,taro),true)

include $(CLEAR_VARS)
LOCAL_MODULE       := ftm_test_config
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES    := config/taro/ftm_test_config
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := ftm_test_config_waipio-qrd-snd-card
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES    := config/taro/ftm_test_config_waipio-qrd-snd-card
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := ftm_test_config_diwali-idp-snd-card
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES    := config/taro/ftm_test_config_diwali-idp-snd-card
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := ftm_test_config_diwali-qrd-snd-card
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES    := config/taro/ftm_test_config_diwali-qrd-snd-card
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := ftm_test_config_diwali-idp-sku1-snd-card
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES    := config/taro/ftm_test_config_diwali-idp-sku1-snd-card
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := ftm_test_config_diwali-qrd-sku1-snd-card
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
LOCAL_SRC_FILES    := config/taro/ftm_test_config_diwali-qrd-sku1-snd-card
include $(BUILD_PREBUILT)
endif

ifeq ($(call is-board-platform-in-list,monaco),true)
ifeq ($(TARGET_SUPPORTS_WEAR_AON),true)
 include $(CLEAR_VARS)
 LOCAL_MODULE       := ftm_test_config
 LOCAL_MODULE_TAGS  := optional
 LOCAL_MODULE_CLASS := ETC
 LOCAL_MODULE_OWNER := qti
 LOCAL_PROPRIETARY_MODULE := true
 LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
 LOCAL_SRC_FILES    := config/monaco/ftm_test_config_slate
 include $(BUILD_PREBUILT)
else
 include $(CLEAR_VARS)
 LOCAL_MODULE       := ftm_test_config
 LOCAL_MODULE_TAGS  := optional
 LOCAL_MODULE_CLASS := ETC
 LOCAL_MODULE_OWNER := qti
 LOCAL_PROPRIETARY_MODULE := true
 LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/
 LOCAL_SRC_FILES    := config/monaco/ftm_test_config
 include $(BUILD_PREBUILT)
endif
endif

endif
