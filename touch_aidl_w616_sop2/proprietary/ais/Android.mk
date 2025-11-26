ifeq ($(strip $(USE_IMAGING_AIS)),true)

LOCAL_PATH := $(call my-dir)
AIS_PATH := $(LOCAL_PATH)

#build 64bit by default
AIS_32_BIT_FLAG := false

#Option to compile all libs statically into ais_server
AIS_BUILD_STATIC := false

#option to disable health monitor
AIS_DISABLE_HEALTH := false

ais_compile_cflags :=

AIS_BUILD_FOR_EARLYSERVICE := false

#Enable this flag to build AIS along with hal3 camera stack
#with this flag IFE0 will be disable from AIS
AIS_BUILD_WITH_HAL_CAMERA := false

#Build ais with CHI cdk to support bayer sensor
AIS_BUILD_WITH_CAMX := true

#Build AIS for early(system) or late(vendor)
ifneq (, $(filter $(BOARD_SUPPORTS_RAMDISK_EARLY_INIT) $(BOARD_SUPPORTS_EARLY_INIT), true))
AIS_BUILD_FOR_EARLYSERVICE := true
ais_compile_cflags += -DAIS_EARLYSERVICE
endif

ifneq (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
ais_compile_cflags += -DANDROID_T_ABOVE
endif


ifeq ($(AIS_BAYER_ISP),true)
ais_compile_cflags += \
	-DMAX9296_DEFAULT_BAYER \
	-DAIS_DEFAULT_FRAMEBASED
endif

ifeq ($(AIS_TI_BOARD),true)
ais_compile_cflags += \
	-DCAMERA_CONFIG_TI9702_DAUGHTER_CARD
endif

ifeq ($(AIS_BUILD_STATIC),true)
ais_compile_cflags += \
	-DAIS_BUILD_STATIC_DEVICES \
	-DAIS_BUILD_STATIC_CONFIG
endif

ifeq ($(TARGET_USES_TV_TUNER),true)
ais_compile_cflags += -DENABLE_TV_TUNER
endif

ifeq ($(AIS_DISABLE_HEALTH),true)
ais_compile_cflags += \
	-DAIS_DISABLE_HEALTH
endif

ifeq ($(AIS_ISP_DISABLE_SHDR),true)
ais_compile_cflags += \
        -DAIS_ISP_DISABLE_SHDR
endif

ifeq ($(AIS_ISP_ENABLE_JPEG),true)
ais_compile_cflags += \
        -DAIS_ISP_ENABLE_JPEG
endif

ais_compile_cflags += -Werror -fvisibility=hidden

ifeq ($(ENABLE_HYP), true)
ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
ais_compile_cflags += -DQCC_SOC_OVERRIDE
endif
endif

include $(AIS_PATH)/libais_log.mk
include $(AIS_PATH)/libais_client.mk

#Currently, not compiling v4l2 based code, since ION buffer needs to be removed
ifneq ($(TARGET_KERNEL_VERSION), 4.14)
include $(AIS_PATH)/hal/ais_v4l2_proxy_v2/Android.mk
else
include $(AIS_PATH)/hal/ais_v4l2_proxy/Android.mk
ais_compile_cflags += \
    -DKERNEL_VERSION4_14
endif


ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
AIS_BUILD_WITH_CPMS_SUPPORT := true
include $(AIS_PATH)/hal/aidl_qti_service/Android.mk
include $(AIS_PATH)/hal/ais_v4l2_camera_hal/Android.mk
include $(AIS_PATH)/hal/evs/aidl/Android.mk
else
include $(AIS_PATH)/hal/v4l2_camera_hal/Android.mk
include $(AIS_PATH)/hal/evs/1.1/Android.mk
endif

#Do not compile hal for early service until sepolicy issue fixed
ifneq (($(AIS_BUILD_FOR_EARLYSERVICE),true) || (,$(filter Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION))))
#Do not compile evs1.0 for Android R
ifneq ( ,$(filter 10 Q, $(PLATFORM_VERSION)))
include $(AIS_PATH)/hal/evs/1.0/Android.mk
else
ifneq ( ,$(filter VanillaIceCream 15, $(PLATFORM_VERSION)))
include $(AIS_PATH)/hal/qcarcam_aidl/Android.mk
else
ifneq ( ,$(filter UpsideDownCake 14, $(PLATFORM_VERSION)))
ifeq ($(ENABLE_HYP), true)
include $(AIS_PATH)/hal/qcarcam_aidl/Android.mk
else
include $(AIS_PATH)/hal/qcarcam/Android.mk
endif
else
include $(AIS_PATH)/hal/qcarcam/Android.mk
endif
endif
endif
endif #AIS_BUILD_FOR_EARLYSERVICE

include $(AIS_PATH)/test/Android.mk

#only compile ais server for non-HYP
ifneq ($(ENABLE_HYP), true)
include $(AIS_PATH)/libais_config.mk
include $(AIS_PATH)/ImagingInputs/Android.mk
include $(AIS_PATH)/libais.mk
include $(AIS_PATH)/ais_server.mk
endif #ENABLE_HYP

include $(AIS_PATH)/qcc6/Android.mk

endif #USE_IMAGING_AIS
