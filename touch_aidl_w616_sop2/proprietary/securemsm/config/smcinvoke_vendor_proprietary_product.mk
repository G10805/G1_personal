
$(call add_soong_config_namespace, minktransport_config)
$(call add_soong_config_namespace, credentials_config)
ifeq ($(TARGET_ENABLE_MINK_COMPONENT),true)
$(call add_soong_config_var_value, minktransport_config, minktransport, enabled)
$(call add_soong_config_var_value, credentials_config, credentials_component, enabled)
else
$(call add_soong_config_var_value, minktransport_config, minktransport, disabled)
$(call add_soong_config_var_value, credentials_config, credentials_component, disabled)
endif

PRODUCT_PACKAGES += libsmcinvoketest_utils
PRODUCT_PACKAGES += smcinvoke_vendor_client
PRODUCT_PACKAGES += libqcbor
PRODUCT_PACKAGES += libminkdescriptor
PRODUCT_PACKAGES += tz_whitelist.json
PRODUCT_PACKAGES += ta_config.json
PRODUCT_PACKAGES += TrustZoneAccessService
PRODUCT_PACKAGES += libnative-api
PRODUCT_PACKAGES += libminksocket_vendor
ifeq ($(strip $(SSGTZD_DAEMON)),true)
PRODUCT_PACKAGES += ssgtzd
PRODUCT_PACKAGES += ssgtzd.rc
PRODUCT_PACKAGES += libtaautoload
PRODUCT_PACKAGES += libSaveQTEEDiag
endif
PRODUCT_PACKAGES += qwes_cli
PRODUCT_PACKAGES += qls_hlos
PRODUCT_PACKAGES += qls_uefi
PRODUCT_PACKAGES += qwesd
ifeq ($(TARGET_BOARD_PLATFORM),sun)
PRODUCT_PACKAGES += libminkipcbinder_vendor
PRODUCT_PACKAGES += vendor.qti.hardware.minkipcbinder@1.0-service
PRODUCT_PACKAGES += vendor.qti.hardware.minkipcbinder-V1-ndk_platform.so
PRODUCT_PACKAGES += vendor.qti.hardware.minkipcbinder@1.0-service.rc
PRODUCT_PACKAGES += vendor.qti.hardware.minkipcbinder-service.xml
endif
