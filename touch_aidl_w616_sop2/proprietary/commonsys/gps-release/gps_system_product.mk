ifneq ($(BOARD_SYSTEM_QCOM_GPS_LOC_API_HARDWARE),)

PRODUCT_PACKAGES += vendor.qti.gnss

ifeq ($(TARGET_DEVICE),apq8026_lw)
LW_FEATURE_SET := true
endif

ifeq ($(TARGET_SUPPORTS_WEAR_OS),true)
LW_FEATURE_SET := true
endif

ifneq ($(LW_FEATURE_SET),true)

PRODUCT_PACKAGES += com.qualcomm.location
PRODUCT_PACKAGES += com.qualcomm.location.xml

PRODUCT_PACKAGES += privapp-permissions-com.qualcomm.location.xml
PRODUCT_PACKAGES += com.qti.location.sdk
PRODUCT_PACKAGES += com.qti.location.sdk.xml
PRODUCT_PACKAGES += liblocsdk_diag_jni

# GPS_DBG
ifneq ($(TARGET_HAS_LOW_RAM),true)
PRODUCT_PACKAGES_DEBUG += com.qualcomm.qti.qlogcat
endif

endif # ifneq ($(LW_FEATURE_SET),true)

PRODUCT_PACKAGES_DEBUG += com.qualcomm.qmapbridge.xml
PRODUCT_PACKAGES_DEBUG += com.qualcomm.qti.izattools.xml
PRODUCT_PACKAGES_DEBUG += ODLT
PRODUCT_PACKAGES_DEBUG += qmapbridge
PRODUCT_PACKAGES_DEBUG += libdiagbridge
PRODUCT_PACKAGES_DEBUG += libloc2jnibridge
PRODUCT_PACKAGES_DEBUG += SampleLocationAttribution

## Feature flags

## Soong Namespace - Keys and values
# Enable/disable location android automotive location features. Default is false.
$(call soong_config_set, qtilocationsys, feature_locauto, false)
$(call soong_config_set, qtilocationsys, feature_locautolegacy, false)

ifeq ($(strip $(TARGET_BOARD_AUTO)),true)
    $(call soong_config_set, qtilocationsys, feature_locauto, true)
    ifeq ($(filter $(TARGET_BOARD_PLATFORM), qssi_au qssi qssi_64),)
        $(call soong_config_set, qtilocationsys, feature_locautolegacy, true)
    endif
endif #TARGET_BOARD_AUTO

endif # BOARD_SYSTEM_QCOM_GPS_LOC_API_HARDWARE
