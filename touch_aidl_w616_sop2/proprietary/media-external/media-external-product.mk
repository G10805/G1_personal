#---------------------------------------------------------------------------------------------------
# HW Codec2.0 HAL compilation
# #-------------------------------------------------------------------------------------------------
TARGET_SUFFIX := $(TARGET_BOARD_PLATFORM)
MSM_VIDC_TARGET_LIST := lahaina holi msmnile sm6150 direwolf
MSM_VIDC_GKI_TARGET_LIST := lahaina holi msmnile sm6150 direwolf
HW_CODEC2_SUPPORTED_TARGET_LIST := lahaina holi msmnile sm6150 direwolf

HW_CODEC2_ENABLED := false

QCV_LAHAINA_FAMILY := lahaina lahaina_default shima_v1 shima_v2 shima_v3 yupik_v0 yupik_v1
QCV_HOLI_FAMILY := holi blair
QCV_MSMNILE_FAMILY := msmnile msmnile_au msmnile_au_km4 msmnile_au_ar msmnile_gvmq msmnile_gvmq_s_u msmnile_au_s_u msmnile_gvmgh msmnile_tb
QTIV_SM6150_FAMILY := sm6150 sm6150_au
QCV_DIREWOLF_FAMILY := direwolf

ifneq ($(filter $(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX),$(QCV_LAHAINA_FAMILY)),)
    QCV_DEVICE_FAMILY := $(QCV_LAHAINA_FAMILY)
endif

ifneq ($(filter $(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX),$(QCV_HOLI_FAMILY)),)
    QCV_DEVICE_FAMILY := $(QCV_HOLI_FAMILY)
endif

ifneq ($(filter $(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX),$(QCV_MSMNILE_FAMILY)),)
    QCV_DEVICE_FAMILY := $(QCV_MSMNILE_FAMILY)
    ifeq ($(ENABLE_HYP),true)
        #For Auto GVM target, enable all xml and json files for all devices to support single lunch combo
        QCV_DEVICE_FAMILY := $(QCV_MSMNILE_FAMILY) $(QCV_DIREWOLF_FAMILY) $(QTIV_SM6150_FAMILY)
    endif
endif

ifneq ($(filter $(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX),$(QTIV_SM6150_FAMILY)),)
    QCV_DEVICE_FAMILY := $(QTIV_SM6150_FAMILY)
endif

ifneq ($(filter $(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX),$(QCV_DIREWOLF_FAMILY)),)
    QCV_DEVICE_FAMILY := $(QCV_DIREWOLF_FAMILY)
endif

ifeq ($(call is-board-platform-in-list, $(HW_CODEC2_SUPPORTED_TARGET_LIST)),true)
  ifeq ($(call is-board-platform-in-list, $(MSM_VIDC_TARGET_LIST)), true)
    $(warning "HW Codec2.0 Enabled")
    HW_CODEC2_ENABLED := true
  endif
endif

ifeq ($(TARGET_USES_QMAA),true)
  ifneq ($(TARGET_USES_QMAA_OVERRIDE_VIDEO),true)
    HW_CODEC2_ENABLED := false
    $(warning "HW Codec2.0 Disabled due to QMAA")
  endif #TARGET_USES_QMAA_OVERRIDE_VIDEO
endif #TARGET_USES_QMAA

$(warning "HW Codec2.0 Enabled = $(HW_CODEC2_ENABLED)")

ifeq ($(HW_CODEC2_ENABLED),true)
    # Target specific data files
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),video_system_specs_$(device).json)
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),media_codecs_$(device).xml)
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),media_codecs_$(device)_va.xml)
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),media_codecs_$(device)_vendor.xml)
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),media_codecs_performance_$(device).xml)
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),media_codecs_performance_$(device)_va.xml)
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),media_codecs_performance_$(device)_vendor.xml)
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),media_profiles_$(device).xml)
    CODEC_MODULES += $(foreach device,$(QCV_DEVICE_FAMILY),media_profiles_$(device)_vendor.xml)
    # Generic data files (GSI)
    CODEC_MODULES += media_profiles_V1_0.xml

    # TODO(SM):
    #  1. remove the _legacy modules once the media-external-product.mk is linked in
    #  2. Clean these from the manifest
    #  3. Remove media_profiles_vendor.xml, .. from media
    CODEC_MODULES += media_profiles_vendor_legacy.xml
    CODEC_MODULES += video_system_specs_legacy.json
    CODEC_MODULES += media_codecs_legacy.xml
    CODEC_MODULES += media_codecs_performance_legacy.xml
    CODEC_MODULES += media_profiles_V1_0_legacy.xml

    # Hooks
    CODEC_MODULES += libqcodec2_hooks

    # Filters
    CODEC_MODULES += libqc2filter
    CODEC_MODULES += libqc2colorconvertfilter
    CODEC_MODULES += libqcodec2_mockfilter

    # Init script
    CODEC_MODULES += init.qti.media.sh
else # QMAA mode
    # Generic data files (GSI)
    CODEC_MODULES += media_profiles_V1_0.xml

    # Legacy media xmls
    CODEC_MODULES += media_profiles_vendor_legacy.xml
    CODEC_MODULES += media_codecs_legacy.xml
    CODEC_MODULES += media_codecs_performance_legacy.xml
endif

PRODUCT_PACKAGES += $(CODEC_MODULES)

#--------------------------------------------------------------------------
# QConfig compilation
# -------------------------------------------------------------------------

QCONFIG_ENABLED := false
QCONFIG_TARGET_LIST := lahaina sm6150 msmnile

ifeq ($(call is-board-platform-in-list, $(QCONFIG_TARGET_LIST)), true)
    QCONFIG_ENABLED := true
endif

$(warning "QCONFIG Enabled = $(QCONFIG_ENABLED)")

ifeq ($(QCONFIG_ENABLED), true)
    # aidl
    QCONFIG_MODULES += vendor.qti.hardware.qconfig-V1-ndk
    # service, client and test
    QCONFIG_MODULES += qconfigservice
    QCONFIG_MODULES += libqconfigclient
    QCONFIG_MODULES += qconfigfunctest
    QCONFIG_MODULES += qconfigpresets.json
endif

ifeq ($(call is-board-platform-in-list, $(HW_CODEC2_SUPPORTED_TARGET_LIST)),true)
    PRODUCT_PROPERTY_OVERRIDES += debug.stagefright.c2inputsurface=-1
endif

PRODUCT_PACKAGES += $(QCONFIG_MODULES)
