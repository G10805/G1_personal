PRODUCT_PACKAGES += libais_log_qcx
PRODUCT_PACKAGES += libqcx_test_util
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
PRODUCT_PACKAGES += libqcx_aidl_client
PRODUCT_PACKAGES += qcarcam_aidl_test_qcx
else
PRODUCT_PACKAGES += libqcx_hidl_client
PRODUCT_PACKAGES += qcarcam_hidl_test_qcx
endif

## EVS HAL common files
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
## EVS AIDL files
PRODUCT_PACKAGES += evs_qcx_aidl_app
PRODUCT_PACKAGES += config_qcarcam_qcx_aidl.json
PRODUCT_PACKAGES += evsmanagerd
PRODUCT_PACKAGES += cardisplayproxyd
else
PRODUCT_PACKAGES += evs_qcx_app
PRODUCT_PACKAGES += config_qcarcam_qcx.json
PRODUCT_PACKAGES += evs_qcx_app_default_resources
endif

# Commenting out evs.manager@1.1 temporarily for Android U
# to support common build infrastructure. AIDL based EVS manager
# will be enabled shortly. EVS app can be used with following
# command "/system/bin/evs_qcx_app --hw --test" to disable use of evs manager.

ifneq ( ,$(filter T Tiramisu 13, $(PLATFORM_VERSION)))
PRODUCT_PACKAGES += android.automotive.evs.manager@1.1
PRODUCT_PACKAGES += libqcx_hidl_client
PRODUCT_PACKAGES += qcarcam_hidl_test_qcx
endif


SOONG_CONFIG_NAMESPACES += camconfigcommonsys
# Soong Keys
SOONG_CONFIG_camconfigcommonsys += camconfig_enabled
# Soong Values

SOONG_CONFIG_camconfigcommonsys_camconfig_enabled := false
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15 W Baklava 16, $(PLATFORM_VERSION)))
SOONG_CONFIG_camconfigcommonsys_camconfig_enabled := true
endif

