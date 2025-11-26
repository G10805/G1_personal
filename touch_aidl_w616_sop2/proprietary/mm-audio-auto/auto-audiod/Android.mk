ifneq ($(ENABLE_HYP),true)
LOCAL_PATH:= $(call my-dir)

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_AUTO_AUDIOD)), true)

AUDIO_HAL_EXT := true
VEHICLE_HAL_EXT := true

include $(CLEAR_VARS)

ifneq ($(filter sdmshrike msmnile gen4,$(TARGET_BOARD_PLATFORM)),)
	LOCAL_CFLAGS := -DPLATFORM_MSMNILE
endif
ifneq ($(filter $(MSMSTEPPE),$(TARGET_BOARD_PLATFORM)),)
	LOCAL_CFLAGS := -DPLATFORM_MSMSTEPPE
endif
ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX), gen4_au)
    LOCAL_CFLAGS := -DPLATFORM_LEMANS -DAGM_HOSTLESS
endif
ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX), msmnile_au)
    LOCAL_CFLAGS := -DPLATFORM_MSMNILE_AU -DAGM_HOSTLESS -DAUDIOD_AMS_CORE_INIT
endif

LOCAL_MODULE:= audiod
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin

LOCAL_SRC_FILES:= \
	auto_audiod_main.cpp \
	AutoAudioDaemon.cpp \
	auto_audio_ext.cpp \
	hostless_cfg.cpp \

ifneq ($(TARGET_BOARD_PLATFORM),$(MSMSTEPPE))
LOCAL_SRC_FILES += \
	auto_audio_ext_v2.cpp
endif

LOCAL_HEADER_LIBRARIES := \
	libagm_headers \

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libbinder \
	libtinyalsa \
	libhidlbase \
	libhidltransport \
	libhwbinder \
	libagmclient \
	libsndcardparser

ifeq ($(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX), msmnile_au)
	LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio-auto/ams/ipc/HwBinder/ams_client_proxy/inc
	LOCAL_SHARED_LIBRARIES += libamsclient
endif

LOCAL_C_INCLUDES += \
	external/tinyalsa/include \

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
         LOCAL_CFLAGS += -DANDROID_U_HAL7
endif

ifeq ($(AUDIO_HAL_EXT), true)
        LOCAL_CFLAGS += -DAHAL_EXT
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
        LOCAL_SHARED_LIBRARIES += android.hardware.audio@7.0 \
                android.hardware.audio.common@7.0
        LOCAL_C_INCLUDES += $(TOP)/android/hardware/audio/7.0
else
        LOCAL_SHARED_LIBRARIES += android.hardware.audio@6.0 \
                android.hardware.audio.common@6.0
        LOCAL_C_INCLUDES += $(TOP)/android/hardware/audio/6.0
endif
endif

ifeq ($(VEHICLE_HAL_EXT), true)
    LOCAL_CFLAGS += -DVHAL_EXT
    LOCAL_SRC_FILES += AutoPower.cpp
    ifeq (12,$(filter 12 S ,$(filter-out . ,$(PLATFORM_VERSION))))
        LOCAL_SHARED_LIBRARIES += android.frameworks.automotive.powerpolicy-V1-ndk_platform
    else
        LOCAL_SHARED_LIBRARIES += android.frameworks.automotive.powerpolicy-V1-ndk
    endif
    LOCAL_SHARED_LIBRARIES += libpowerpolicyclient \
                              libbinder_ndk
endif

ifeq ($(AUDIO_FEATURE_ENABLED_SILENT_BOOT), true)
        LOCAL_C_INCLUDES += vendor/qcom/opensource/audio-hal/primary-hal/hal \
                            system/media/audio/include
        LOCAL_CFLAGS += -DSILENT_BOOT_ENABLED
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
        LOCAL_CFLAGS += -DSILENT_BOOT_ANDROID_U
endif
endif

ifneq ( ,$(filter V VanillaIceCream 15, $(PLATFORM_VERSION)))

	LOCAL_CFLAGS += -DAOSP_UPGRADE_CHANGES
endif

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_EXECUTABLE)

endif
endif # ENABLE_HYP

