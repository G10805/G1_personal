ifeq ($(strip $(USE_IMAGING_AIS)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

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
ifeq ($(BOARD_SUPPORTS_EARLY_INIT),true)
ifeq ( ,$(filter 10 Q 11 R, $(PLATFORM_VERSION)))
AIS_BUILD_FOR_EARLYSERVICE := true
ais_compile_cflags += -DAIS_EARLYSERVICE
endif
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

ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
#Build ais with CPMS PowerEvents Support for HALs
AIS_BUILD_WITH_CPMS_SUPPORT := true
ifeq ($(AIS_BUILD_WITH_CPMS_SUPPORT),true)
include $(AIS_PATH)/hal/power/Android.mk
endif #AIS_BUILD_WITH_CPMS_SUPPORT
endif #PLATFORM_VERSION

ifeq ($(ENABLE_HYP), true)
ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
ais_compile_cflags += -DQCC_SOC_OVERRIDE

include $(AIS_PATH)/libais_log.mk
include $(AIS_PATH)/libais_client.mk

ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
include $(AIS_PATH)/hal/ais_v4l2_camera_hal/Android.mk
include $(AIS_PATH)/hal/qcarcam_aidl/Android.mk
include $(AIS_PATH)/hal/evs/aidl/Android.mk
else
include $(AIS_PATH)/hal/ais_v4l2_proxy_v2/Android.mk
include $(AIS_PATH)/hal/v4l2_camera_hal/Android.mk
include $(AIS_PATH)/hal/qcarcam/Android.mk
include $(AIS_PATH)/hal/evs/1.1/Android.mk
endif

include $(AIS_PATH)/test/Android.mk

include $(AIS_PATH)/hal/v4l2_camera_hal_wrapper/Android.mk
endif #PLATFORM_VERSION
endif #ENABLE_HYP

#disable below component for QCC6, support is not present
#only compile ais server for non-HYP
#ifneq ($(ENABLE_HYP), true)
#include $(AIS_PATH)/libais_config.mk
#include $(AIS_PATH)/ImagingInputs/Android.mk
#include $(AIS_PATH)/libais.mk
#include $(AIS_PATH)/ais_server.mk
#endif #ENABLE_HYP



endif #USE_IMAGING_AIS
