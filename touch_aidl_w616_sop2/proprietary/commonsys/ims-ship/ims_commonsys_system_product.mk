ifneq ($(ENABLE_HYP), true)
ifneq ($(TARGET_SUPPORTS_WEARABLES), true)

IMS_SHIP_RCS += ImsRcsService
IMS_SHIP_RCS += whitelist_com.qualcomm.qti.uceShimService
IMS_SHIP_RCS += uceShimService
IMS_SHIP_RCS += vendor.qti.ims.rcsservice.xml
IMS_SHIP_RCS += ImsDataChannelService
IMS_SHIP_RCS += vendor.qti.imsdcservice.xml
IMS_SHIP_RCS += datachannellib
IMS_SHIP_RCS += datachannellib.xml

PRODUCT_PACKAGES += $(IMS_SHIP_RCS)

endif

#IMS_SHIP_VT += lib-imscamera
IMS_SHIP_VT += lib-imsvideocodec

PRODUCT_PACKAGES += $(IMS_SHIP_VT)

endif
