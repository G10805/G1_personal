ANDROID_R_SDK_VERSION = 30
CURRENT_SDK_VERSION = ${PLATFORM_SDK_VERSION}
ifeq ($(PLATFORM_VERSION), R)
	CURRENT_SDK_VERSION = $(ANDROID_R_SDK_VERSION)
endif

CHECK_VERSION_G = if [ $(1) -g $(2) ] ; then echo true ; else echo false ; fi

# Compile the project only for versions greater than Android R
ifeq ($(call CHECK_VERSION_G, $(CURRENT_SDK_VERSION), $(ANDROID_R_SDK_VERSION)), true)

LOCAL_PATH := $(call my-dir)
LOCAL_C_INCLUDES     := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(call all-subdir-makefiles)

C2AUDIO_USE_STUB_PAL := false
C2AUDIO_DEBUG_MODE := false
#==================================================================================================
# Temporarily build modules with Android.mk till kernel headers are available in soong
# TODO(PC): wipe this out and re-enable Android.bp once 'kernel-headers' header_library is available
#==================================================================================================
#--------------------------------------------------------------------------------------------------
# Customization/debugging hooks
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqc2audio_hooks
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional

SRC_PATH := hooks

LOCAL_SRC_FILES      := \
    $(SRC_PATH)/QC2ComponentObserver.cpp \
    $(SRC_PATH)/QC2PerfController.cpp \

LOCAL_C_INCLUDES := \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/vidc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES :=  \
    libcodec2_headers \
    libqc2audio_base_headers \
    libqc2audio_hooks_headers \
    libhardware_headers \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libcodec2_vndk \
    libqc2audio_base \

LOCAL_CFLAGS := -Wall -Werror -std=c++17 -Wno-error=deprecated-declarations -Wno-enum-compare \
                            -fexceptions $(CODEC_FLAGS)
LOCAL_CLANG             := true
include $(BUILD_SHARED_LIBRARY)

#--------------------------------------------------------------------------------------------------
# qc2audio_platform
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqc2audio_platform
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional

SRC_PATH := platform/target/android/

LOCAL_SRC_FILES      := \
    $(SRC_PATH)/QC2PlatformIonInfo.cpp \

LOCAL_C_INCLUDES     := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES :=  \
    libcodec2_headers \
    libqc2audio_base_headers \
    libhardware_headers \
    libqc2audio_platform_headers \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libcodec2_vndk \
    libqdMetaData \
    libqc2audio_base \

LOCAL_CFLAGS := -Wall -Werror -Wno-error=deprecated-declarations -Wno-enum-compare \

LOCAL_CLANG             := true
include $(BUILD_SHARED_LIBRARY)

#--------------------------------------------------------------------------------------------------
# audio swcodec library
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqc2audio_swaudiocodec
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional

SRC_PATH := codecs/sw

LOCAL_SRC_FILES      := \
    $(SRC_PATH)/QC2AudioSwCodec.cpp \
    $(SRC_PATH)/dec/alac/src/QC2AudioSwAlacDec.cpp \
    $(SRC_PATH)/dec/ape/src/QC2AudioSwApeDec.cpp \
    $(SRC_PATH)/dec/speech/src/QC2AudioSwSpeechCommonDec.cpp \
    $(SRC_PATH)/dec/speech/src/QC2AudioSwEvrcDec.cpp \
    $(SRC_PATH)/dec/speech/src/QC2AudioSwQcelpDec.cpp \
    $(SRC_PATH)/dec/flac/src/QC2AudioSwFlacDec.cpp \
    $(SRC_PATH)/dec/dsd/src/QC2AudioSwDsdDec.cpp \
    $(SRC_PATH)/QC2AudioSwCodecPlugin.cpp

LOCAL_HEADER_LIBRARIES := \
    libcodec2_headers \
    libqc2audio_base_headers \
    libqc2audio_basecodec_headers \
    libqc2audio_utils_headers \
    libqc2audio_swcodec_headers \
    libhardware_headers \
    libqtiflacdec_csim_headers

LOCAL_WHOLE_STATIC_LIBRARIES := libqc2audio_swaudiocodec_data_common

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libqc2audio_base \
    libqc2audio_platform \
    libqc2audio_utils \
    libqc2audio_basecodec \
    libcodec2_vndk \

LOCAL_CFLAGS := -Wall -Werror -Wno-error=deprecated-declarations -Wno-enum-compare \
                            $(CODEC_FLAGS) $(EXT_FLAGS)
LOCAL_CLANG             := true
include $(BUILD_SHARED_LIBRARY)

#--------------------------------------------------------------------------------------------------
# swcodec codec entries and capabilities
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqc2audio_swaudiocodec_data_common
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional

SRC_PATH := codecs/sw/target/android/common

LOCAL_SRC_FILES      := \
    $(SRC_PATH)/QC2AudioSwCodecEntries.cpp \
    $(SRC_PATH)/QC2AudioSwCodecCaps.cpp

LOCAL_HEADER_LIBRARIES :=  \
    libcodec2_headers \
    libqc2audio_base_headers \
    libqc2audio_platform_headers \
    libqc2audio_utils_headers \
    libqc2audio_basecodec_headers \
    libqc2audio_swcodec_headers \
    libhardware_headers

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libqc2audio_base \
    libqc2audio_platform \
    libqc2audio_utils \
    libqc2audio_basecodec \
    libcodec2_vndk \

LOCAL_CFLAGS := -Wall -Werror -Wno-error=deprecated-declarations -Wno-enum-compare \

LOCAL_CLANG             := true
include $(BUILD_STATIC_LIBRARY)

ifeq ($(C2AUDIO_USE_STUB_PAL), true)
#--------------------------------------------------------------------------------------------------
# pal stub library
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libpal
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional

SRC_PATH := codecs/hw/dsp/Pal-stub

LOCAL_SRC_FILES      := \
    $(SRC_PATH)/PalApi.cpp

LOCAL_HEADER_LIBRARIES := \
    libqc2audio_pal_headers \

#LOCAL_C_INCLUDES := \
#    $(SRC_PATH)/include

LOCAL_CFLAGS := -Wall -Werror -Wno-error=deprecated-declarations -Wno-enum-compare \
                            $(CODEC_FLAGS) $(EXT_FLAGS)
LOCAL_CLANG             := true
include $(BUILD_SHARED_LIBRARY)
endif
#--------------------------------------------------------------------------------------------------
# audio hwcodec library
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqc2audio_hwaudiocodec
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional

SRC_PATH := codecs/hw

LOCAL_SRC_FILES      := \
    $(SRC_PATH)/QC2AudioHwCodec.cpp \
    $(SRC_PATH)/dec/aac/src/QC2AudioHwAacDec.cpp \
    $(SRC_PATH)/dec/amr/src/QC2AudioHwAmrDec.cpp \
    $(SRC_PATH)/dec/amr/src/QC2AudioHwAmrWbPlusDec.cpp \
    $(SRC_PATH)/dec/alac/src/QC2AudioHwAlacDec.cpp \
    $(SRC_PATH)/dec/ape/src/QC2AudioHwApeDec.cpp \
    $(SRC_PATH)/dec/wma/src/QC2AudioHwWmaDec.cpp \
    $(SRC_PATH)/enc/amr/src/QC2AudioHwAmrNbWbEnc.cpp \
    $(SRC_PATH)/enc/aac/src/QC2AudioHwAacEnc.cpp \
    $(SRC_PATH)/enc/evrc/src/QC2AudioHwEvrcEnc.cpp \
    $(SRC_PATH)/enc/qcelp/src/QC2AudioHwQcelpEnc.cpp \
    $(SRC_PATH)/QC2AudioHwCodecPlugin.cpp \
    $(SRC_PATH)/dsp/QC2AudioHw.cpp \
    $(SRC_PATH)/dsp/QC2AudioHwElite.cpp \
    $(SRC_PATH)/dsp/QC2AudioHwGecko.cpp

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES := \
    libcodec2_headers \
    libqc2audio_base_headers \
    libqc2audio_platform_headers \
    libqc2audio_utils_headers \
    libqc2audio_basecodec_headers \
    libqc2audio_hwcodec_headers \
    libhardware_headers \

LOCAL_WHOLE_STATIC_LIBRARIES := libqc2audio_hwaudiocodec_data_common

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libqc2audio_base \
    libqc2audio_platform \
    libqc2audio_utils \
    libqc2audio_basecodec \
    libcodec2_vndk \

LOCAL_HEADER_LIBRARIES += libstagefright_headers
LOCAL_SHARED_LIBRARIES += libstagefright_foundation

ifeq ($(C2AUDIO_USE_STUB_PAL), true)
LOCAL_HEADER_LIBRARIES += libqc2audio_pal_headers
LOCAL_SHARED_LIBRARIES += libpal
else
LOCAL_C_INCLUDES     := $(TOP)/vendor/qcom/opensource/pal

LOCAL_HEADER_LIBRARIES  += libacdb_headers libspf-headers libarosal_headers
LOCAL_SHARED_LIBRARIES  += libpalclient
endif

LOCAL_CFLAGS := -Wall -Werror -Wno-error=deprecated-declarations -Wno-enum-compare \
                            $(CODEC_FLAGS) $(EXT_FLAGS)
LOCAL_CLANG             := true
include $(BUILD_SHARED_LIBRARY)

#--------------------------------------------------------------------------------------------------
# hwcodec codec entries and capabilities
#--------------------------------------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE         := libqc2audio_hwaudiocodec_data_common
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional

SRC_PATH := codecs/hw/target/android/common

#LOCAL_SRC_FILES      := \
#    $(SRC_PATH)/QC2PlatformCaps_$(TARGET_BOARD_PLATFORM).cpp \
#    $(SRC_PATH)/QC2V4L2CodecEntries_$(TARGET_BOARD_PLATFORM).cpp \

LOCAL_SRC_FILES      := \
    $(SRC_PATH)/QC2AudioHwCodecEntries.cpp \
    $(SRC_PATH)/QC2AudioHwCodecCaps.cpp

LOCAL_C_INCLUDES     :=  $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES :=  \
    libcodec2_headers \
    libqc2audio_base_headers \
    libqc2audio_platform_headers \
    libqc2audio_utils_headers \
    libqc2audio_basecodec_headers \
    libqc2audio_hwcodec_headers \
    libhardware_headers

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libqc2audio_base \
    libqc2audio_platform \
    libqc2audio_utils \
    libqc2audio_basecodec \
    libcodec2_vndk \

LOCAL_CFLAGS := -Wall -Werror -Wno-error=deprecated-declarations -Wno-enum-compare \

LOCAL_CLANG             := true
include $(BUILD_STATIC_LIBRARY)

#--------------------------------------------------------------------------------------------------
# qc2audio 1.0 hal service
#--------------------------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE         := vendor.qti.media.c2audio@1.0-service

LOCAL_MODULE_TAGS           := optional
LOCAL_MODULE_RELATIVE_PATH  := hw
LOCAL_PROPRIETARY_MODULE    := true
LOCAL_MODULE_OWNER          := qti
LOCAL_CLANG                 := true

LOCAL_INIT_RC := service/1.0/vendor.qti.media.c2audio@1.0-service.rc

LOCAL_VINTF_FRAGMENTS := service/1.0/c2_manifest_vendor_audio.xml

LOCAL_SRC_FILES := service/1.0/QC2HalService_1.0.cpp

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_HEADER_LIBRARIES :=  \
    libcodec2_headers \
    libqc2audio_base_headers \
    libhardware_headers \
    libqc2audio_platform_headers \
    libqc2audio_utils_headers \
    libqc2audio_core_api_headers \
    libqc2audio_basecodec_headers \

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libcodec2_vndk \
    libqc2audio_base \
    libqc2audio_utils \
    libqc2audio_basecodec \
    libqc2audio_platform \
    libqc2audio_core \
    libqdMetaData \
    android.hardware.media.c2@1.0 \
    libavservices_minijail \
    libbinder \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libvndksupport \
    libcodec2_hidl@1.0 \

LOCAL_CFLAGS           := -Wall -Werror -Wno-error=deprecated-declarations $(EXT_FLAGS)

include $(BUILD_EXECUTABLE)

endif ##ifeq ($(call CHECK_VERSION_G, $(CURRENT_SDK_VERSION), $(ANDROID_R_SDK_VERSION)), true)
