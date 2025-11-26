ifneq ($(BOARD_SYSTEM_QCOM_GPS_LOC_API_HARDWARE),)
ifneq ($(BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET),true)

GNSS_CFLAGS := \
    -Werror \
    -Wno-error=unused-parameter \
    -Wno-error=macro-redefined \
    -Wno-error=reorder \
    -Wno-error=missing-braces \
    -Wno-error=self-assign \
    -Wno-error=enum-conversion \
    -Wno-error=logical-op-parentheses \
    -Wno-error=null-arithmetic \
    -Wno-error=null-conversion \
    -Wno-error=parentheses-equality \
    -Wno-error=undefined-bool-conversion \
    -Wno-error=tautological-compare \
    -Wno-error=switch

ifeq ($(TARGET_DEVICE),apq8026_lw)
LW_FEATURE_SET := true
endif

ifeq ($(TARGET_SUPPORTS_WEAR_OS),true)
LW_FEATURE_SET := true
endif

LOCAL_PATH := $(call my-dir)

ifneq ($(LW_FEATURE_SET),true)
include $(LOCAL_PATH)/*/Android.mk
endif # LW_FEATURE_SET

endif # BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET
endif # BOARD_SYSTEM_QCOM_GPS_LOC_API_HARDWARE
