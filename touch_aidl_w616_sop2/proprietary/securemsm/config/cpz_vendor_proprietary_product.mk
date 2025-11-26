ifeq ($(strip $(ENABLE_SECCAM)),true)
#No seccam lib/bin in /vendor yet (except for hal service)
ifeq ($(strip $(ENABLE_SECCAM_QTI_SERVICE)),true)
SECUREMSM_SECCAM := vendor.qti.hardware.seccam@1.0.vendor
SECUREMSM_SECCAM += vendor.qti.hardware.seccam@1.0_vendor
SECUREMSM_SECCAM += vendor.qti.hardware.seccam@1.0-service-qti
SECUREMSM_SECCAM += vendor.qti.hardware.seccam@1.0-service-qti.rc
endif
endif

ifeq ($(strip $(ENABLE_TRUSTED_UI_2_0)),true)
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0.vendor
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0_vendor
endif

ifeq ($(strip $(ENABLE_TRUSTED_UI_AIDL)),true)
SECUREMSM_SECDISP += libsi
SECUREMSM_SECDISP += libloadtrusteduiapp
SECUREMSM_SECDISP += TrustedUISampleTestAIDL
SECUREMSM_SECDISP += vendor.qti.hardware.trustedui-V1-ndk
SECUREMSM_SECDISP += vendor.qti.hardware.trustedui-aidl-service-qti
SECUREMSM_SECDISP += vendor.qti.hardware.trustedui-aidl-service-qti.rc
SECUREMSM_SECDISP += vendor.qti.hardware.trustedui-aidl-service.xml
SECUREMSM_SECDISP += libTrustedInputAIDL
SECUREMSM_SECDISP += libTrustedUIAIDL
endif

ifeq ($(strip $(ENABLE_TRUSTED_UI_VM_3_0)), true)
SECUREMSM_SECDISP += TrustedUISampleTAClient
SECUREMSM_SECDISP += trusteduilistener
SECUREMSM_SECDISP += trusteduilistener.rc
SECUREMSM_SECDISP += libTrustedInputUtils
endif

ifeq ($(strip $(ENABLE_SECCAM_2_0_AIDL)),true)
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.common-V1-ndk.so
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.common-V1-ndk.so.vendor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.common-V1-ndk.so_vendor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.config-V1-ndk.so
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.config-V1-ndk.so.vendor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.config-V1-ndk.so_vendor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.device-V1-ndk.so
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.device-V1-ndk.so.vendor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.device-V1-ndk.so_vendor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.common-helper.so
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.common-helper.so.vendor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.common-helper.so_vendor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.xml
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.rc
endif

ifeq ($(strip $(ENABLE_C2PA)),true)
SECUREMSM_C2PA := C2PAInternetService
SECUREMSM_C2PA += C2PANativeTestClient
SECUREMSM_C2PA += C2PATestClientAIDL
SECUREMSM_C2PA += c2painternetservice.rc
SECUREMSM_C2PA += c2pa_enroll_config.json
SECUREMSM_C2PA += c2pa_network_config.json
SECUREMSM_C2PA += c2pa_network.policy
SECUREMSM_C2PA += libc2pa_tag_header
SECUREMSM_C2PA += vendor.qti.hardware.c2pa-aidl-service-qti
SECUREMSM_C2PA += vendor.qti.hardware.c2pa-aidl-service-qti.rc
SECUREMSM_C2PA += vendor.qti.hardware.c2pa-aidl-service.xml
SECUREMSM_C2PA += vendor.qti.hardware.c2pa-V1-ndk
endif

PRODUCT_PACKAGES += $(SECUREMSM_SECCAM)
PRODUCT_PACKAGES += $(SECUREMSM_SECDISP)
PRODUCT_PACKAGES += $(SECUREMSM_C2PA)

# If either ENABLE_PLAYREADY_DRM or ENABLE_WIDEVINE_DRM is true
ifeq (true,$(filter true,$(strip $(ENABLE_PLAYREADY_DRM)) $(strip $(ENABLE_WIDEVINE_DRM))))
SECUREMSM_COMMON_DRM := libdrmtime
SECUREMSM_COMMON_DRM += libdrmfs
SECUREMSM_COMMON_DRM += libtrustedapploader
SECUREMSM_COMMON_DRM += libminkdescriptor
SECUREMSM_COMMON_DRM += libcpion
SECUREMSM_COMMON_DRM += libops
SECUREMSM_COMMON_DRM += libhdcpsrm
SECUREMSM_COMMON_DRM += hdcp_srm
SECUREMSM_COMMON_DRM += libhdcp1prov
SECUREMSM_COMMON_DRM += hdcp1prov
SECUREMSM_COMMON_DRM += libhdcp2p2prov
SECUREMSM_COMMON_DRM += hdcp2p2prov
endif

ifeq ($(strip $(ENABLE_WIDEVINE_DRM)), true)
SECUREMSM_WIDEVINE_DRM := liboemcrypto
SECUREMSM_WIDEVINE_DRM += StoreKeybox
SECUREMSM_WIDEVINE_DRM += InstallKeybox
SECUREMSM_WIDEVINE_DRM += fsp_client
SECUREMSM_WIDEVINE_DRM += $(SECUREMSM_COMMON_DRM)

ifeq ($(strip $(ENABLE_DRM_SAMPLE_APP)), true)
SECUREMSM_WIDEVINE_DRM += drm_sample_app
endif

# Don't use dynamic DRM HAL for non-go SPs

# Add static and dynamic android.hardware.drm-service.widevine
# & android.hardware.drm-service.clearkey to PRODUCT_PACKAGES.

# Reomve this target specific check once google updated APEX binary for wideivne 19 changes.
ifeq ($(TARGET_BOARD_PLATFORM),sun)
SECUREMSM_WIDEVINE_DRM += android.hardware.drm-service.widevine
else ifeq ($(call math_gt_or_eq, $(SHIPPING_API_LEVEL),34), true)
# Integrating Widevine APEX from SHIPPING_API_LEVEL 34(Android_U)
TARGET_BUILD_WIDEVINE := nonupdatable
TARGET_BUILD_WIDEVINE_USE_PREBUILT := true
-include vendor/widevine/libwvdrmengine/apex/device/device.mk
else
SECUREMSM_WIDEVINE_DRM += android.hardware.drm-service.widevine
endif
SECUREMSM_WIDEVINE_DRM += android.hardware.drm-service.clearkey

ifeq ($(strip $(OTA_FLAG_FOR_DRM)),true)
SECUREMSM_WIDEVINE_DRM += move_widevine_data.sh
endif

#The flag makes other components to distinguish and use updated and modified field of WideVine.
SECUREMSM_UPDATED_WIDEVINE_USED := true

#ENABLE_WIDEVINE_DRM
endif

# These static and dynamic (lazy) services are needed for complete
# Widevine DRM use case on full Android-S & S-Go devices.
ifeq ($(TARGET_HAS_LOW_RAM),true)
TARGET_BUILD_WIDEVINE := nonupdatable
TARGET_BUILD_WIDEVINE_USE_PREBUILT := true
-include vendor/widevine/libwvdrmengine/apex/device/device.mk
SECUREMSM_WIDEVINE_DRM += android.hardware.drm-service-lazy.clearkey
endif

#
# Drminitialization is invoked during setDataSource and as a part
# initialization, DrmManagerClient is created which tries to get
# DrmManagerService(“drm.drmManager”).
# Earlier, if the service is not yet started & if the property
# “drm.service.enabled” is not set, then getService call returns NULL
# immediately and client creates a No-op-DrmClientManager.
# But now, AOSP change has disabled this service by default and it will
# only be started if the property  drm.service.enabled is set.
# Hence setting this property in vendor shippable config.mk
#
PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled = true

ifeq ($(strip $(ENABLE_HLOS_DATA_PATH)), true)
PRODUCT_PROPERTY_OVERRIDES += vendor.wv.oemcrypto.debug.enable_hlos_data_path = true
endif

PRODUCT_PACKAGES += $(SECUREMSM_WIDEVINE_DRM)

ifeq ($(strip $(ENABLE_PLAYREADY_DRM)), true)
SECUREMSM_PLAYREADY_DRM := drmtest
SECUREMSM_PLAYREADY_DRM += libdrmMinimalfs
SECUREMSM_PLAYREADY_DRM += libbase64
SECUREMSM_PLAYREADY_DRM += libprpk4
SECUREMSM_PLAYREADY_DRM += libtzdrmgenprov
SECUREMSM_PLAYREADY_DRM += libprdrmengine
ifeq ($(ENABLE_PLAYREADY_AIDL_SERVICE),true)
SECUREMSM_PLAYREADY_DRM += android.hardware.drm-service.playready-qti
SECUREMSM_PLAYREADY_DRM += android.hardware.drm-service.playready-qti.rc
SECUREMSM_PLAYREADY_DRM += libpraidl
else
SECUREMSM_PLAYREADY_DRM += android.hardware.drm@1.3-service.playready-qti
SECUREMSM_PLAYREADY_DRM += android.hardware.drm@1.3-service.playready-qti.rc
SECUREMSM_PLAYREADY_DRM += libprhidl
endif
SECUREMSM_PLAYREADY_DRM += prdrmkeyprov
SECUREMSM_PLAYREADY_DRM += $(SECUREMSM_COMMON_DRM)
SECUREMSM_PLAYREADY_DRM_DEBUG += prdrm_gtest
endif

PRODUCT_PACKAGES += $(SECUREMSM_PLAYREADY_DRM)
PRODUCT_PACKAGES_DEBUG += $(SECUREMSM_PLAYREADY_DRM_DEBUG)

#Add drm script if PLAYREADY_USES_CE_SMMU or WIDEVINE_USES_CE_SMMU is true
# This script will be present on all builds.
# However, it is only triggered for Automotive platforms currently.
ifneq (, $(filter true, $(PLAYREADY_USES_CE_SMMU) $(WIDEVINE_USES_CE_SMMU)))
PRODUCT_PACKAGES += init.drm.smmu.sh
endif

ifeq ($(strip $(ENABLE_PERIPHERAL_STATE_UTILS)), true)
SECUREMSM_PERIPHERAL_SECURITY := libPeripheralStateUtils
endif

PRODUCT_PACKAGES += $(SECUREMSM_PERIPHERAL_SECURITY)
