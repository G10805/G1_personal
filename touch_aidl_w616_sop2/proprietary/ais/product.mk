ifeq ($(strip $(USE_IMAGING_AIS)),true)
## AIS bins and libs for final image
PRODUCT_PACKAGES += ais_server
PRODUCT_PACKAGES += libais
PRODUCT_PACKAGES += libais_log_proprietary
PRODUCT_PACKAGES += libais_client
PRODUCT_PACKAGES += libais_config
PRODUCT_PACKAGES += libais_max9296
PRODUCT_PACKAGES += libais_max9296b
PRODUCT_PACKAGES += libais_tids90ub
PRODUCT_PACKAGES += libais_sensorstub
## qcarcam test
#PRODUCT_PACKAGES_DEBUG += qcarcam_test
#PRODUCT_PACKAGES_DEBUG += qcarcam_diag
#PRODUCT_PACKAGES += qcarcam_edrm_rvc
#PRODUCT_PACKAGES += libais_test_util_edrm
#PRODUCT_PACKAGES += libais_test_util_edrm_proprietary
#PRODUCT_PACKAGES += qcarcam_edrm_rvc.rc
#PRODUCT_PACKAGES += libais_test_util_proprietary
#PRODUCT_PACKAGES += 1cam.xml
#PRODUCT_PACKAGES += 1cam_pproc_nv12.xml
#PRODUCT_PACKAGES += 8cam.xml

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
## EVS AIDL specific files
#PRODUCT_PACKAGES += android.hardware.automotive.evs-ais
#PRODUCT_PACKAGES += android.frameworks.automotive.display@1.0-service
## Camera2 AIDL HAL
#PRODUCT_PACKAGES += aidl.device-impl
#PRODUCT_PACKAGES += aidl.provider-impl
#PRODUCT_PACKAGES += android.hardware.camera.provider@V1-qti-service
#PRODUCT_PACKAGES += vendor.qti.camera.provider-service.rc
#PRODUCT_PACKAGES += vendor.qti.camera.provider.xml
#PRODUCT_PACKAGES += ais_config_v4l2_camera_hal.json
#PRODUCT_PACKAGES += libais_power
ifneq ( ,$(filter V VanillaIceCream 15, $(PLATFORM_VERSION)))
## QCARCAM AIDL
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@V1-service
endif
ifneq ( ,$(filter U UpsideDownCake 14, $(PLATFORM_VERSION)))
ifeq ($(ENABLE_HYP), true)
## QCARCAM AIDL
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@V1-service
endif
endif
else
## EVS1.1 HAL specific files
#PRODUCT_PACKAGES += android.automotive.evs.manager@1.1
#PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1
#PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1-ais
#PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1-java
#PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1-impl
#PRODUCT_PACKAGES += evs_sample_configuration_ais.xml
#PRODUCT_PACKAGES += evs_configuration_ais.dtd
#PRODUCT_PACKAGES += evs_ais_hal_resources
## Automotive display service for EVS1.1
PRODUCT_PACKAGES += android.frameworks.automotive.display@1.0-service
## AIS v4l2 proxy service
PRODUCT_PACKAGES += ais_v4l2_proxy
PRODUCT_PACKAGES += ais_v4l2loopback_config.xml
# enable qcarcam_v4l2_test
#PRODUCT_PACKAGES_DEBUG += qcarcam_v4l2_test
## Camera2 HIDL HAL
#PRODUCT_PACKAGES += config_v4l2_camera_hal.json
# Camera configuration file. Shared by passthrough/binderized camera HAL
#PRODUCT_PACKAGES += camera.device@3.4-impl
#PRODUCT_PACKAGES += camera.device@3.4-external-impl
# Enable binderized camera HAL for full hal3/v4l2 hal
#PRODUCT_PACKAGES += android.hardware.camera.provider@2.4-external
#PRODUCT_PACKAGES += android.hardware.camera.provider@2.4-legacy
#PRODUCT_PACKAGES += android.hardware.camera.provider@2.4-impl
#PRODUCT_PACKAGES += android.hardware.camera.provider@2.4-service
endif # PLATFORM_VERSION
## QCARCAM HAL
##PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@1.0
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@1.1
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@1.0-java
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@1.0-service
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@1.0-impl

## AIS v4l2 hal libs
ifeq ($(ENABLE_HYP), true)
ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
#PRODUCT_PACKAGES += camera.v4l2.qcc4
else
#PRODUCT_PACKAGES += camera.v4l2
endif
else
#PRODUCT_PACKAGES += camera.v4l2
endif # ENABLE_HYP
# override with v4l2 hal
PRODUCT_PROPERTY_OVERRIDES += ro.hardware.camera=v4l2
# enable ccidbgr
PRODUCT_PACKAGES_DEBUG += ccidbgr
endif #USE_IMAGING_AIS

## Re-enable CameraService for Android R
ifneq ( ,$(filter 11 R ,$(PLATFORM_VERSION)))
PRODUCT_PROPERTY_OVERRIDES += config.disable_cameraservice=false
endif

