ifneq ($(strip $(USE_CAMERA_STUB)),true)

ifneq ($(CAMX_CHICDK_PATH),)
LOCAL_PATH := $(CAMX_CHICDK_PATH)/vendor
else
LOCAL_PATH := $(call my-dir)
endif

# INclude this for definintions
include $(CAMX_CHICDK_PATH)/vendor/common.mk

ifeq ($(TARGET_BOARD_PLATFORM),atoll)
include $(CAMX_VENDOR_PATH)/chioverride/default/atoll/build/android/Android.mk
endif

ifneq ( ,$(filter $(MSMSTEPPE) $(MSMSTEPPE)_au, $(TARGET_BOARD_PLATFORM)))
include $(CAMX_VENDOR_PATH)/chioverride/default/sm6150/build/android/Android.mk

#excluding Moorea target, In Auto we are not using Moorea
#include $(CAMX_VENDOR_PATH)/chioverride/default/moorea/build/android/Android.mk
endif
ifneq ( ,$(filter msmnile msmnile_au msmnile_au_km4 msmnile_au_ar msmnile_tb sdmshrike_au, $(TARGET_BOARD_PLATFORM)))
include $(CAMX_VENDOR_PATH)/chioverride/default/msmnile/build/android/Android.mk
endif
ifeq ($(TARGET_BOARD_PLATFORM),sdm845)
include $(CAMX_VENDOR_PATH)/chioverride/default/sdm845/build/android/Android.mk
endif
ifeq ($(TARGET_BOARD_PLATFORM),sdm710)
include $(CAMX_VENDOR_PATH)/chioverride/default/sdm845/build/android/Android.mk
endif
ifeq ($(TARGET_BOARD_PLATFORM),qcs605)
include $(CAMX_VENDOR_PATH)/chioverride/default/sdm845/build/android/Android.mk
endif
include $(CAMX_VENDOR_PATH)/node/chiutils/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/depth/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/gpu/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/memcpy/build/android/Android.mk
ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),$(MSMSTEPPE) $(MSMSTEPPE)_au atoll))
include $(CAMX_VENDOR_PATH)/node/swcac/build/android/Android.mk
endif
include $(CAMX_VENDOR_PATH)/node/dummysat/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/dummyrtb/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/remosaic/build/android/Android.mk

#no fastcv for now
#include $(CAMX_VENDOR_PATH)/node/fcv/build/android/Android.mk


include $(CAMX_VENDOR_PATH)/node/dummystich/build/android/Android.mk
ifeq ( ,$(filter msmnile_tb, $(TARGET_BOARD_PLATFORM)))
include $(CAMX_VENDOR_PATH)/sensor/default/max9296_ar0231/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/sensor/default/max9296_ar0231rt/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/sensor/default/ti960_ox03a10/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/sensor/default/max9296_ar0820/build/android/Android.mk
endif
include $(CAMX_VENDOR_PATH)/sensor/default/max9296_ar0820rt/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/staticpdlibalgo/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/staticaecalgo/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/staticafalgo/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/staticawbalgo/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/awbwrapper/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/aecwrapper/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/afwrapper/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/hafoverride/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/hvx/addconstant/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/hvx/binning/build/android/Android.mk
include $(CAMX_VENDOR_PATH)/node/shdr/build/android/Android.mk


ifneq ($(ANDROID_FLAVOR),)
ifeq ($(ANDROID_FLAVOR),$(filter $(ANDROID_FLAVOR),$(ANDROID_FLAVOR_P) $(ANDROID_FLAVOR_Q)))
ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),$(MSMSTEPPE) $(MSMSTEPPE)_au atoll))
include $(CAMX_VENDOR_PATH)/node/dewarp/build/android/Android.mk
endif
endif # ($(ANDROID_FLAVOR),$(filter $(ANDROID_FLAVOR),$(ANDROID_FLAVOR_P) $(ANDROID_FLAVOR_Q)))
endif #($(ANDROID_FLAVOR),)
endif #!USE_CAMERA_STUB
