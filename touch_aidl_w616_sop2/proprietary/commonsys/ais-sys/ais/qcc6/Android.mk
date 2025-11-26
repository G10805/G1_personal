ifeq ($(strip $(USE_IMAGING_AIS)),true)

LOCAL_PATH := $(call my-dir)
AIS_PATH := $(LOCAL_PATH)

#build 64bit by default
AIS_32_BIT_FLAG := false

#Option to compile all libs statically into ais_server
AIS_BUILD_STATIC := false

ais_compile_cflags :=

AIS_BUILD_FOR_EARLYSERVICE := false

#Build AIS for early(system) or late(vendor)
ifeq ($(BOARD_SUPPORTS_EARLY_INIT),true)
ifeq ( ,$(filter 10 Q 11 R, $(PLATFORM_VERSION)))
AIS_BUILD_FOR_EARLYSERVICE := true
ais_compile_cflags += -DAIS_EARLYSERVICE
endif
endif

ifeq ($(AIS_BUILD_STATIC),true)
ais_compile_cflags += \
	-DAIS_BUILD_STATIC_DEVICES \
	-DAIS_BUILD_STATIC_CONFIG
endif

ifeq ($(TARGET_USES_TV_TUNER),true)
ais_compile_cflags += -DENABLE_TV_TUNER
endif

ais_compile_cflags += -Werror -fvisibility=hidden

include $(AIS_PATH)/libais_log.mk

ifeq ($(PLATFORM_VERSION),$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
#Build ais with CPMS PowerEvents Support for Apps
AIS_BUILD_WITH_CPMS_SUPPORT := true
endif #PLATFORM_VERSION

#Do not compile hal for early service until sepolicy issue fixed
ifneq ($(AIS_BUILD_FOR_EARLYSERVICE),true)
include $(AIS_PATH)/hal/ais_qcarcam_aidl/Android.mk
include $(AIS_PATH)/hal/ais_qcarcam_hidl/Android.mk
endif #AIS_BUILD_FOR_EARLYSERVICE

include $(AIS_PATH)/test/Android.mk
include $(AIS_PATH)/test/evs/1.1/Android.mk

endif #USE_IMAGING_AIS
