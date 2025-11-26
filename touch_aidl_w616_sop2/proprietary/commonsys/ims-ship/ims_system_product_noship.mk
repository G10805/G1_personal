#IMS_VT
ifneq ($(ENABLE_HYP), true)

IMS_VT := lib-imsvt
IMS_VT += lib-imsvtutils
IMS_VT += lib-imsvtextutils

PRODUCT_PACKAGES += $(IMS_VT)

ifneq ($(TARGET_HAS_LOW_RAM),true)
    MSTAT := MStatsSystemService
    PRODUCT_PACKAGES += $(MSTAT)
endif

endif
