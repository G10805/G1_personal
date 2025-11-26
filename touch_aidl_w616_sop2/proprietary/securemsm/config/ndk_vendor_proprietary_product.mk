#Include ndks
ifeq ($(INCLUDE_NDK), true)
PRODUCT_PACKAGES += android.hardware.security.keymint-V3-ndk
PRODUCT_PACKAGES += android.hardware.security.keymint-V3-ndk.vendor
PRODUCT_PACKAGES += android.hardware.security.rkp-V3-ndk
PRODUCT_PACKAGES += android.hardware.security.rkp-V3-ndk.vendor
PRODUCT_PACKAGES += android.hardware.security.secureclock-V1-ndk
PRODUCT_PACKAGES += android.hardware.security.secureclock-V1-ndk.vendor
PRODUCT_PACKAGES += android.hardware.security.sharedsecret-V1-ndk
PRODUCT_PACKAGES += android.hardware.security.sharedsecret-V1-ndk.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.qseecom-V1-ndk
PRODUCT_PACKAGES += vendor.qti.hardware.qseecom-V1-ndk.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.qteeconnector-V1-ndk
PRODUCT_PACKAGES += vendor.qti.hardware.qteeconnector-V1-ndk.vendor
PRODUCT_PACKAGES += android.hardware.gatekeeper-V1-ndk
PRODUCT_PACKAGES += android.hardware.gatekeeper-V1-ndk.vendor
PRODUCT_PACKAGES += libgatekeeper
PRODUCT_PACKAGES += libgatekeeper.vendor
endif
