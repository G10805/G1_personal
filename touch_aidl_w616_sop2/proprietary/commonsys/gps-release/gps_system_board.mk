# Any TARGET_BOARD_PLATFORM being built that does
# not want system location modules built should be
# added to this exclude list to prevent building
LOC_BOARD_PLATFORM_EXCLUDE_LIST :=

#QSPA will be set to true only for qssi_lite variants
ifeq ($(TARGET_USES_QSPA), true)
    EXCLUDE_LOCATION_FEATURES := true
endif

# Define BOARD_SYSTEM_QCOM_GPS_LOC_API_HARDWARE if:
# EXCLUDE_LOCATION_FEATURES is not true AND
# TARGET_BOARD_PLATFORM is not in LOC_BOARD_PLATFORM_EXCLUDE_LIST AND
# (TARRGET_USES_QMMA is not true OR
#  TARGET_USES_QMAA_OVERRIDE_GPS is not false
ifneq ($(EXCLUDE_LOCATION_FEATURES),true)
  ifeq (,$(filter $(LOC_BOARD_PLATFORM_EXCLUDE_LIST),$(TARGET_BOARD_PLATFORM)))
    ifneq ($(TARGET_USES_QMAA),true)
      BOARD_SYSTEM_QCOM_GPS_LOC_API_HARDWARE := default
    else ifneq ($(TARGET_USES_QMAA_OVERRIDE_GPS),false)
      BOARD_SYSTEM_QCOM_GPS_LOC_API_HARDWARE := default
    endif
  endif #LOC_BOARD_PLATFORM_EXCLUDE_LIST check
endif #EXCLUDE_LOCATION_FEATURES check

ifneq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)
  BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET := true
else
# Enable PDK feature for Auto Android-T PLs when the product supports value add.
  ifeq ($(TARGET_BOARD_AUTO), true)
     ifeq ($(PLATFORM_SDK_VERSION), 33)
         BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET := true
     else
         BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET := false
     endif
  else
     BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET := false
  endif
endif
