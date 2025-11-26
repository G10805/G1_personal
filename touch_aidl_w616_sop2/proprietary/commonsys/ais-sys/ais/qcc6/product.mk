PRODUCT_PACKAGES += libais_log_qcc6
PRODUCT_PACKAGES += libais_test_util_qcc6
PRODUCT_PACKAGES += libais_aidl_client_qcc6
PRODUCT_PACKAGES += qcarcam_aidl_test_qcc6
PRODUCT_PACKAGES += qcarcam_hidl_test_qcc6
PRODUCT_PACKAGES += libais_hidl_client_qcc6
ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
PRODUCT_PACKAGES += libais_sys_power
PRODUCT_PACKAGES += android.frameworks.automotive.display@1.0-service
endif #PLATFORM_VERSION

ifneq ( ,$(filter T Tiramisu 13, $(PLATFORM_VERSION)))
$(call soong_config_set,power,power_driver_android_version,android_t)
endif

## EVS HAL common files
ifeq ($(ENABLE_HYP), true)
ifneq ( ,$(filter T Tiramisu 13, $(PLATFORM_VERSION)))
PRODUCT_PACKAGES += evs_ais_app_qcc6
PRODUCT_PACKAGES += config_qcarcam_qcc6.json
PRODUCT_PACKAGES += evs_ais_app_default_resources_qcc6
PRODUCT_PACKAGES += android.automotive.evs.manager@1.1
endif
endif
