ifneq ($(TARGET_HAS_LOW_RAM),true)
#CapabilityConfigStore HIDL

CCS_HIDL := vendor.qti.hardware.capabilityconfigstore@1.0
CCS_HIDL += vendor.qti.hardware.capabilityconfigstore-V1.0-java
CCS_HIDL += vendor.qti.hardware.capabilityconfigstore@1.0-impl
CCS_HIDL += vendor.qti.hardware.capabilityconfigstore@1.0-service
CCS_HOST += configstore_xmlparser

CCS_HIDL_DBG := capabilityconfigstoretest

PRODUCT_PACKAGES += $(CCS_HIDL)
PRODUCT_HOST_PACKAGES += $(CCS_HOST)
PRODUCT_PACKAGES_DEBUG += $(CCS_HIDL_DBG)

endif
