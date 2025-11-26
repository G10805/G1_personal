LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES += common/AVConfigHelper.cpp
LOCAL_SRC_FILES += common/AVLog.cpp

LOCAL_SRC_FILES += media/ExtendedMediaUtils.cpp

LOCAL_SRC_FILES += stagefright/ExtendedFactory.cpp
LOCAL_SRC_FILES += stagefright/ExtendedUtils.cpp
LOCAL_SRC_FILES += stagefright/ExtendedACodec.cpp
LOCAL_SRC_FILES += stagefright/ExtendedAudioSource.cpp
LOCAL_SRC_FILES += stagefright/ExtendedCameraSource.cpp
LOCAL_SRC_FILES += stagefright/ExtendedLiveSession.cpp
LOCAL_SRC_FILES += stagefright/ExtendedPlaylistFetcher.cpp
LOCAL_SRC_FILES += stagefright/ExtendedCameraSourceTimeLapse.cpp
LOCAL_SRC_FILES += stagefright/CompressAACAudioSource.cpp

LOCAL_SRC_FILES += mediaplayerservice/ExtendedNuFactory.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedNuPlayer.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedNuUtils.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedNuPlayerDecoderPassThrough.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedNuPlayerDecoder.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedNuPlayerRenderer.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedServiceFactory.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedSFRecorder.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedServiceUtils.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedARTSPConnection.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedARTPConnection.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedWriter.cpp
LOCAL_SRC_FILES += mediaplayerservice/WAVEWriter.cpp
LOCAL_SRC_FILES += mediaplayerservice/ExtendedHTTPLiveSource.cpp

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/av/media/libavextensions \
        $(TOP)/frameworks/av/media/libmediaplayerservice \
        $(TOP)/frameworks/av/media/libstagefright \
        $(TOP)/frameworks/av/media/libstagefright/mpeg2ts \
        $(TOP)/frameworks/av/media/libstagefright/httplive \
        $(TOP)/system/media/audio/include/system/ \
        $(TOP)/external/flac/include \
        $(TOP)/system/media/audio_utils/include \
        $(TOP)/frameworks/av/media/libstagefright/rtsp \
        $(LOCAL_PATH)/include

LOCAL_HEADER_LIBRARIES := \
        libmedia_headers \
        libmediametrics_headers \
        libstagefright_foundation_headers \
        media_plugin_headers \
        libmediadrm_headers \
        libstagefright_rtsp_headers \
        libstagefright_mpeg2support_headers

LOCAL_SHARED_LIBRARIES := \
        libpermission \
        libaudioclient_aidl_conversion \
        libandroid_net \
        libbase \
        libnetd_client \
        libbinder \
        libcamera_client \
        libcutils \
        libdl \
        libexpat \
        liblog \
        libutils \
        libz \
        libmedia \
        libmedia_omx \
        libhidlbase \
        libstagefright \
        libstagefright_codecbase \
        libstagefright_httplive \
        libmediaplayerservice \
        libstagefright_foundation \
        libaudioutils \
        libaudioclient \
        libaudiopolicy \
        libnativewindow \
        libmedia_helper \
        libcrypto \
        libhidlmemory \
        android.hidl.allocator@1.0 \
        android.hidl.memory@1.0 \
        libdatasource \
        libnbaio

LOCAL_STATIC_LIBRARIES := \
        libstagefright_rtsp \
        libstagefright_mpeg2support_nocrypto

#QTI Decoder/offload flags
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_EXTN_FLAC_DECODER)),true)
LOCAL_CFLAGS := -DQTI_FLAC_DECODER
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_FLAC_OFFLOAD)),true)
LOCAL_CFLAGS += -DFLAC_OFFLOAD_ENABLED
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_ALAC_OFFLOAD)),true)
LOCAL_CFLAGS += -DALAC_OFFLOAD_ENABLED
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_WMA_OFFLOAD)), true)
LOCAL_CFLAGS += -DWMA_OFFLOAD_ENABLED
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_APE_OFFLOAD)),true)
LOCAL_CFLAGS += -DAPE_OFFLOAD_ENABLED
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_VORBIS_OFFLOAD)),true)
LOCAL_CFLAGS += -DVORBIS_OFFLOAD_ENABLED
endif
endif

ifeq ($(call is-platform-sdk-version-at-least,27),true) # O-MR1
LOCAL_CFLAGS += -D_ANDROID_O_MR1_CHANGES
endif

LOCAL_CFLAGS += -Wno-multichar -Werror

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_PCM_OFFLOAD)),true)
   LOCAL_CFLAGS += -DPCM_OFFLOAD_ENABLED_16
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_PCM_OFFLOAD_24)),true)
   LOCAL_CFLAGS += -DPCM_OFFLOAD_ENABLED_24
endif

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_AAC_ADTS_OFFLOAD)),true)
   LOCAL_CFLAGS += -DAAC_ADTS_OFFLOAD_ENABLED
endif

ENABLE_HLS_AUDIO_ONLY_IMG_DISPLAY := false
ifeq ($(ENABLE_HLS_AUDIO_ONLY_IMG_DISPLAY), true)
LOCAL_CFLAGS += -DHLS_AUDIO_ONLY_IMG_DISPLAY
LOCAL_C_INCLUDES += $(TOP)/external/jpeg
LOCAL_SHARED_LIBRARIES += libjpeg
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libui
endif

LOCAL_CFLAGS += -DBRINGUP_WIP
LOCAL_CFLAGS += -DUSE_CAMERA_METABUFFER_UTILS

ifeq ($(call is-platform-sdk-version-at-least,27),true) # O-MR1
LOCAL_CFLAGS += -D_ANDROID_O_MR1
endif

ifdef LLVM_REPORT_DIR
LOCAL_CFLAGS      += --compile-and-analyze $(LLVM_REPORT_DIR)
LOCAL_CPPFLAGS    += --compile-and-analyze $(LLVM_REPORT_DIR)
endif

LOCAL_SANITIZE += cfi
LOCAL_SYSTEM_EXT_MODULE := true

LOCAL_MODULE:= libavenhancements

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
