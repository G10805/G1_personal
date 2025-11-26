# Secure Camera 2.0 binaries
ifneq ($(TARGET_BOARD_AUTO),true)
SECUREMSM_SECCAM_DBG += Cam2test
endif
SECUREMSM_SECCAM += com.qti.media.secureprocessor
SECUREMSM_SECCAM += libmediasp_jni
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.common@1.0
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.config@1.0
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.device@1.0
SECUREMSM_SECCAM += vendor.qti.hardware.secureprocessor.common@1.0-helper
SECUREMSM_SECCAM += com.qti.media.secureprocessor.xml

# TrustedUI 2.0 binaries
ifneq ($(TARGET_BOARD_AUTO),true)
SECUREMSM_SECDISP += com.qualcomm.qti.services.systemhelper
endif
SECUREMSM_SECDISP += libsystemhelper_jni
SECUREMSM_SECDISP += vendor.qti.hardware.systemhelper@1.0

# Add to PRODUCT_PACKAGES
PRODUCT_PACKAGES += $(SECUREMSM_SECCAM)
PRODUCT_PACKAGES += $(SECUREMSM_SECDISP)
PRODUCT_PACKAGES_DEBUG += $(SECUREMSM_SECCAM_DBG)
