PRODUCT_PACKAGES += libais_log
PRODUCT_PACKAGES += qcarcam_hidl_test
PRODUCT_PACKAGES += libais_test_util
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
PRODUCT_PACKAGES += libais_aidl_client
PRODUCT_PACKAGES += qcarcam_aidl_test
endif
PRODUCT_PACKAGES += libais_hidl_client
## EVS AIDL files
PRODUCT_PACKAGES += evs_qcom_app
PRODUCT_PACKAGES += config_qcar.json
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15 W Baklava 16, $(PLATFORM_VERSION)))
PRODUCT_PACKAGES += evsmanagerd
PRODUCT_PACKAGES += cardisplayproxyd
PRODUCT_PACKAGES += libais_sys_power
PRODUCT_PACKAGES += android.frameworks.automotive.display@1.0-service
else
PRODUCT_PACKAGES += android.automotive.evs.manager@1.1
endif
## EVS HIDL APP common files
PRODUCT_PACKAGES += evs_ais_app
PRODUCT_PACKAGES += config_qcarcam.json
PRODUCT_PACKAGES += evs_ais_app_default_resources
PRODUCT_PACKAGES += CarFromTop_qcar.png
PRODUCT_PACKAGES += LabeledChecker_qcar.png
#enable CarEvsPreview JAVA app
PRODUCT_PACKAGES += CarEvsCameraPreviewApp
PRODUCT_PACKAGES += SampleRearViewCamera
PRODUCT_PACKAGE_OVERLAYS += packages/services/Car/tests/SampleRearViewCamera/overlay

SOONG_CONFIG_NAMESPACES += camconfigcommonsys
# Soong Keys
SOONG_CONFIG_camconfigcommonsys += camconfig_enabled
# Soong Values

SOONG_CONFIG_camconfigcommonsys_camconfig_enabled := false
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15 W Baklava 16, $(PLATFORM_VERSION)))
SOONG_CONFIG_camconfigcommonsys_camconfig_enabled := true
endif
