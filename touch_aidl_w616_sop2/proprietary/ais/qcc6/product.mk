ifeq ($(strip $(USE_IMAGING_AIS)),true)
## AIS bins and libs for final image
#PRODUCT_PACKAGES += ais_server_qcc6
#PRODUCT_PACKAGES += libais_qcc6
#PRODUCT_PACKAGES += libais_log_proprietary_qcc6
#PRODUCT_PACKAGES += libais_client_qcc6
#PRODUCT_PACKAGES += libais_config_qcc6
#PRODUCT_PACKAGES += libais_max9296_qcc6
#PRODUCT_PACKAGES += libais_max9296b_qcc6
#PRODUCT_PACKAGES += libais_tids90ub_qcc6
#PRODUCT_PACKAGES += libais_sensorstub_qcc6
## qcarcam test
#PRODUCT_PACKAGES += qcarcam_test_qcc6
# PRODUCT_PACKAGES += qcarcam_diag_qcc6
# PRODUCT_PACKAGES += qcarcam_v4l2_test_qcc6
#ifneq ($(TARGET_KERNEL_VERSION),$(filter 6.1,$(TARGET_KERNEL_VERSION)))
#PRODUCT_PACKAGES += qcarcam_edrm_rvc_qcc6
#PRODUCT_PACKAGES += libais_test_util_edrm_qcc6
#PRODUCT_PACKAGES += libais_test_util_edrm_proprietary_qcc6
#PRODUCT_PACKAGES += qcarcam_edrm_rvc_qcc6.rc
#endif
#PRODUCT_PACKAGES += libais_test_util_proprietary_qcc6
#PRODUCT_PACKAGES += 1cam.xml
#PRODUCT_PACKAGES += 1cam_pproc_nv12_qcc6.xml
#PRODUCT_PACKAGES += 8cam.xml
#PRODUCT_PACKAGES += 1cam_v4l2.xml
#PRODUCT_PACKAGES += 8cam_v4l2.xml

ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
ifeq ($(AIS_BUILD_WITH_CPMS_SUPPORT),true)
PRODUCT_PACKAGES += libais_power
endif
endif

#ifeq ($(ENABLE_HYP), true)
#ifneq ( ,$(filter U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
## EVS AIDL specific files
#PRODUCT_PACKAGES += android.hardware.automotive.evs-ais-qcc6
PRODUCT_PACKAGES += android.frameworks.automotive.display@1.0-service
## QCARCAM AIDL HAL
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam2@V1-service-qcc6
#PRODUCT_PACKAGES += ais_config_v4l2_camera_hal_qcc6.json
#else
## EVS1.1 HAL specific files
#PRODUCT_PACKAGES += android.automotive.evs.manager@1.1
#PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1
#PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1-ais-qcc6
#PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1-java
#PRODUCT_PACKAGES += android.hardware.automotive.evs@1.1-impl
#PRODUCT_PACKAGES += evs_sample_configuration_ais.xml
#PRODUCT_PACKAGES += evs_configuration_ais.dtd
#PRODUCT_PACKAGES += evs_ais_hal_resources
#PRODUCT_PACKAGES += CarFromTop_qcar_qcc6.png
#PRODUCT_PACKAGES += LabeledChecker_qcar_qcc6.png
## Automotive display service for EVS1.1
#PRODUCT_PACKAGES += android.frameworks.automotive.display@1.0-service
## QCARCAM HIDL HAL
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@2.0-qcc6
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@2.0-java-qcc6
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@2.0-service-qcc6
#PRODUCT_PACKAGES += vendor.qti.automotive.qcarcam@2.0-impl-qcc6
## AIS v4l2 proxy service
#PRODUCT_PACKAGES += ais_v4l2_proxy_qcc6
#PRODUCT_PACKAGES += ais_v4l2loopback_config.xml
## AIS v4l2 hal libs
#PRODUCT_PACKAGES += config_v4l2_camera_hal_qcc6.json
#endif #PLATFORM_VERSION
#ifeq ($(PLATFORM_VERSION),$(filter T Tiramisu 13 U UpsideDownCake 14 V VanillaIceCream 15, $(PLATFORM_VERSION)))
#PRODUCT_PACKAGES += camera.v4l2
#PRODUCT_PACKAGES += camera.v4l2.qcc6
#endif
#endif #ENABLE_HYP

# override with v4l2 hal
PRODUCT_PROPERTY_OVERRIDES += ro.hardware.camera=v4l2
endif #USE_IMAGING_AIS

## Re-enable CameraService for Android R
ifneq ( ,$(filter 11 R ,$(PLATFORM_VERSION)))
PRODUCT_PROPERTY_OVERRIDES += config.disable_cameraservice=false
endif
