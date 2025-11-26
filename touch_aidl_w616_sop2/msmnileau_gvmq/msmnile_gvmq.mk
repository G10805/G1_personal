TARGET_BOARD_PLATFORM := msmnile
TARGET_BOOTLOADER_BOARD_NAME := msmnile
TARGET_BOARD_TYPE := auto
TARGET_BOARD_SUFFIX := _gvmq
ENABLE_AIDL_VHAL := true
ENABLE_AIDL_SENSOR := true
# U-BRINGUP disable display
TARGET_DISABLE_DISPLAY := false
TARGET_IS_HEADLESS := false
TARGET_DISABLE_CODEC2 := true
TARGET_DISABLE_VPP_FILTER := true
TARGET_DISABLE_HSI2S_DLKM := true
TARGET_DISABLE_DISPLAY_DLKM := false
TARGET_DISABLE_AIS_DLKM := true
TARGET_DISABLE_LIBVIRTDIAG := true

SHIPPING_API_LEVEL := 34
PRODUCT_SHIPPING_API_LEVEL := $(SHIPPING_API_LEVEL)
BOARD_SHIPPING_API_LEVEL := $(SHIPPING_API_LEVEL)

AUDIO_USE_STUB_HAL := false
# Skip VINTF checks for kernel configs since we do not have kernel source
PRODUCT_OTA_ENFORCE_VINTF_KERNEL_REQUIREMENTS := false
PRODUCT_MANUFACTURER := Qualcomm
PRODUCT_DEVICE := msmnile_gvmq

ifeq ($(TARGET_SINGLE_TREE), true)
  PRODUCT_PRODUCT_VNDK_VERSION := current
  #TODO(amutyala) to revert once QSSI 15 component created
  #This change requires to build super image (QSSI15 + V14)
  ifeq (,$(filter VanillaIceCream V 35, $(PLATFORM_VNDK_VERSION)))
    PRODUCT_EXTRA_VNDK_VERSIONS := 33
  else
    PRODUCT_EXTRA_VNDK_VERSIONS := 33 34
  endif

  PRODUCT_ENFORCE_PRODUCT_PARTITION_INTERFACE := true

  # Enable debugfs restrictions
  PRODUCT_SET_DEBUGFS_RESTRICTIONS := true

  PRODUCT_SOONG_NAMESPACES += \
      frameworks/base/boot \
      cts/tests/signature/api-check \
      hardware/google/av \
      hardware/google/interfaces

  TARGET_USES_NEW_ION := true

  TARGET_USES_AOSP_FOR_AUDIO := false

  # Audio configuration file
  #-include $(TOPDIR)vendor/qcom/opensource/audio-hal/primary-hal/configs/qssi/qssi.mk
  #-include $(TOPDIR)vendor/qcom/opensource/commonsys/audio/configs/qssi/qssi.mk
  AUDIO_FEATURE_ENABLED_SVA_MULTI_STAGE := true
endif

PRODUCT_VENDOR_PROPERTIES += \
    ro.soc.manufacturer=$(PRODUCT_MANUFACTURER) \


PRODUCT_VENDOR_PROPERTIES += \
    apexd.config.dm_create.timeout=3000 \

# Touch HAL for Dream service
PRODUCT_PACKAGES += \
    vendor.visteon.touch-service

# Enable support for APEX updates
$(call inherit-product, $(SRC_TARGET_DIR)/product/updatable_apex.mk)

ALLOW_MISSING_DEPENDENCIES := true
ENABLE_AB ?= true
# Enable virtual-ab by default
ifeq ($(ENABLE_AB), true)
  ENABLE_VIRTUAL_AB ?= true
endif
ifeq ($(ENABLE_VIRTUAL_AB), true)
  ifeq ($(TARGET_SINGLE_TREE), true)
    $(call inherit-product, $(SRC_TARGET_DIR)/product/generic_ramdisk.mk)
  endif
  ifeq (true,$(call math_gt_or_eq,$(SHIPPING_API_LEVEL),34))
    # For OTA updates with shipping api level 34 and above.
    $(call inherit-product, $(SRC_TARGET_DIR)/product/virtual_ab_ota/vabc_features.mk)
    $(call inherit-product, $(SRC_TARGET_DIR)/product/generic_ramdisk.mk)
    PRODUCT_VENDOR_PROPERTIES += ro.virtual_ab.compression.threads=true
  else
    # For OTA updates with shipping api level 33 and below.
    $(call inherit-product, $(SRC_TARGET_DIR)/product/generic_ramdisk.mk)
    $(call inherit-product, $(SRC_TARGET_DIR)/product/virtual_ab_ota/android_t_baseline.mk)
  endif
  PRODUCT_VIRTUAL_AB_COMPRESSION_METHOD := gz
endif
# Enable AVB 2.0
BOARD_AVB_ENABLE := true
BOARD_USES_QCNE := false
TARGET_BOARD_AUTO := true
TARGET_USES_AOSP := true
#TODO(amutyala) to revert this once QSSI 15 component created
ifeq (,$(filter VanillaIceCream V 35, $(PLATFORM_VNDK_VERSION)))
  TARGET_USES_GAS := true
endif
TARGET_USES_QCOM_BSP := false
TARGET_NO_TELEPHONY := true
TARGET_USES_QTIC := false
TARGET_USES_QTIC_EXTENSION := false
ENABLE_HYP := true
TARGET_CONSOLE_ENABLED ?= true
# FR77687: Migrate AIDL interface using -ndk_platform.so to -ndk.so
NEED_AIDL_NDK_PLATFORM_BACKEND := true
TARGET_NO_QTI_WFD := true
BOARD_HAVE_QCOM_FM := false
BOARD_VENDOR_QCOM_LOC_PDK_FEATURE_SET := false
TARGET_ENABLE_QC_AV_ENHANCEMENTS := false
TARGET_FWK_SUPPORTS_AV_VALUEADDS := true
#TARGET_FWK_SUPPORTS_FULL_VALUEADDS := false
ifeq ($(TARGET_SINGLE_TREE), true)
  TARGET_FWK_SUPPORTS_FULL_VALUEADDS := true
endif
TARGET_USES_AOSP_FOR_WLAN := true
# U-BRINGUP disable wlan
BOARD_HAS_QCOM_WLAN := false
QCOM_WLAN_FOR_AUTO_GVM := true
ENABLE_CAR_POWER_MANAGER := true
VPP_TARGET_USES_SERVICE := NO
ENABLE_AUDIO_LEGACY_TECHPACK := true
TARGET_USES_QCOM_MM_AUDIO := true
TARGET_GVMGH_SPECIFIC := false

TARGET_USES_RRO := true
TARGET_HAS_VIRTIO_FASTRPC := true

#Enable Userspace Restart
$(call inherit-product, $(SRC_TARGET_DIR)/product/userspace_reboot.mk)


# Dynamic-partition enabled by default
BOARD_DYNAMIC_PARTITION_ENABLE := true
ifeq ($(strip $(BOARD_DYNAMIC_PARTITION_ENABLE)),true)
  PRODUCT_USE_DYNAMIC_PARTITIONS := true
  BOARD_BUILD_SUPER_IMAGE_BY_DEFAULT := false
  PRODUCT_BUILD_SUPER_PARTITION := false
  PRODUCT_BUILD_RAMDISK_IMAGE := true
  PRODUCT_PACKAGES += fastbootd
   # Add default implementation of fastboot AIDL.
  PRODUCT_PACKAGES += android.hardware.fastboot-service.example_recovery

PRODUCT_PACKAGES += \
        android.hardware.automotive.vehicle@V1-visteon-service

#DIAG HAL        
PRODUCT_PACKAGES += \
        vendor.visteon.hardware.automotive.diagnostics@V1-visteon-service

  # Mismatch in the uses-library tags between build system and the manifest leads
  # to soong APK manifest_check tool errors. Enable the flag to fix this.
  RELAX_USES_LIBRARY_CHECK := true

  ifeq ($(ENABLE_AB), true)
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/6155_ufs/fstab_AB_dynamic_partition_variant.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.gen3.ufs.qcom
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/6155_emmc/fstab_AB_dynamic_partition_variant.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.gen3.emmc.qcom
    ifeq (true,$(call math_gt_or_eq,$(SHIPPING_API_LEVEL),34))
      PRODUCT_COPY_FILES += $(LOCAL_PATH)/gen4_fstab_metadata_f2fs/fstab_AB_dynamic_partition_variant.gen4.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.qcom
      PRODUCT_COPY_FILES += $(LOCAL_PATH)/gen4_fstab_metadata_f2fs/fstab_AB_dynamic_partition_variant.gen4.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.gen4.qcom
    else
      PRODUCT_COPY_FILES += $(LOCAL_PATH)/fstab_AB_dynamic_partition_variant.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.qcom
      PRODUCT_COPY_FILES += $(LOCAL_PATH)/fstab_AB_dynamic_partition_variant.gen4.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.gen4.qcom
    endif
  else
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/6155_ufs/fstab_non_AB_dynamic_partition_variant.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.gen3.ufs.qcom
    PRODUCT_COPY_FILES += $(LOCAL_PATH)/6155_emmc/fstab_non_AB_dynamic_partition_variant.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.gen3.emmc.qcom
    ifeq (true,$(call math_gt_or_eq,$(SHIPPING_API_LEVEL),34))
      PRODUCT_COPY_FILES += $(LOCAL_PATH)/gen4_fstab_metadata_f2fs/fstab_non_AB_dynamic_partition_variant.gen4.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.qcom
      PRODUCT_COPY_FILES += $(LOCAL_PATH)/gen4_fstab_metadata_f2fs/fstab_non_AB_dynamic_partition_variant.gen4.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.gen4.qcom
    else
      PRODUCT_COPY_FILES += $(LOCAL_PATH)/fstab_non_AB_dynamic_partition_variant.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.qcom
      PRODUCT_COPY_FILES += $(LOCAL_PATH)/fstab_non_AB_dynamic_partition_variant.gen4.qti:$(TARGET_COPY_OUT_VENDOR_RAMDISK)/first_stage_ramdisk/fstab.gen4.qcom
    endif
  endif
endif

PRODUCT_BUILD_SYSTEM_IMAGE := false
# Enable System_ext
PRODUCT_BUILD_SYSTEM_EXT_IMAGE := false
PRODUCT_BUILD_PRODUCT_IMAGE := false
TARGET_SKIP_OTA_PACKAGE := true
ifeq ($(TARGET_SINGLE_TREE), true)
  PRODUCT_BUILD_SYSTEM_IMAGE := true
  PRODUCT_BUILD_SYSTEM_EXT_IMAGE := true
  PRODUCT_BUILD_PRODUCT_IMAGE := true
  TARGET_SKIP_OTA_PACKAGE := false
  BOARD_BUILD_SUPER_IMAGE_BY_DEFAULT := true
  PRODUCT_BUILD_SUPER_PARTITION := true
endif
PRODUCT_BUILD_SYSTEM_OTHER_IMAGE := false
#PRODUCT_BUILD_VENDOR_IMAGE := true
PRODUCT_BUILD_PRODUCT_SERVICES_IMAGE := false
#PRODUCT_BUILD_ODM_IMAGE := true
PRODUCT_BUILD_CACHE_IMAGE := false
PRODUCT_BUILD_RAMDISK_IMAGE := true
PRODUCT_BUILD_USERDATA_IMAGE := true
PRODUCT_BUILD_VENDOR_BOOT_IMAGE := true
PRODUCT_BUILD_VENDOR_DLKM_IMAGE := true
PRODUCT_BUILD_SYSTEM_DLKM_IMAGE := true

#Using sha256 for dm-verity partitions.
#system, system_other, system_ext and product.
#TODO @asmemoha cleanup syste/system_ext avb configs. No impact either way
BOARD_AVB_SYSTEM_ADD_HASHTREE_FOOTER_ARGS += --hash_algorithm sha256
BOARD_AVB_SYSTEM_EXT_ADD_HASHTREE_FOOTER_ARGS += --hash_algorithm sha256
ifeq ($(TARGET_SINGLE_TREE), true)
  BOARD_AVB_PRODUCT_ADD_HASHTREE_FOOTER_ARGS += --hash_algorithm sha256
endif
BOARD_AVB_VENDOR_ADD_HASHTREE_FOOTER_ARGS += --hash_algorithm sha256
BOARD_AVB_SYSTEM_DLKM_ADD_HASHTREE_FOOTER_ARGS += --hash_algorithm sha256
BOARD_AVB_VENDOR_DLKM_ADD_HASHTREE_FOOTER_ARGS += --hash_algorithm sha256

ifneq ("$(wildcard device/qcom/$(TARGET_BOARD_PLATFORM)-kernel/vendor_dlkm/system_dlkm.modules.blocklist)", "")
PRODUCT_COPY_FILES += device/qcom/$(TARGET_BOARD_PLATFORM)-kernel/vendor_dlkm/system_dlkm.modules.blocklist:$(TARGET_COPY_OUT_VENDOR_DLKM)/lib/modules/system_dlkm.modules.blocklist
endif

TARGET_DEFINES_DALVIK_HEAP := true
# Disable 32bit App support.
# This value should be set before including device/qcom/common/common64.mk
DEVICE_SUPPORTS_64_BIT_APPS_ONLY := true
$(call inherit-product, device/qcom/common/common64.mk)
#Inherit all except heap growth limit from phone-xhdpi-2048-dalvik-heap.mk
PRODUCT_PROPERTY_OVERRIDES  += \
   dalvik.vm.heapstartsize=8m \
   dalvik.vm.heapsize=512m \
   dalvik.vm.heaptargetutilization=0.75 \
   dalvik.vm.heapminfree=512k \
   dalvik.vm.heapmaxfree=8m \
   vendor.gatekeeper.disable_spu = true #\


PRODUCT_PROPERTY_OVERRIDES += ro.control_privapp_permissions=enforce

$(call inherit-product, packages/services/Car/car_product/build/car.mk)

PRODUCT_NAME := msmnile_gvmq
PRODUCT_BRAND := qti
PRODUCT_MODEL := msmnile_gvmq for arm64
TARGET_PRODUCT_OEM := W616_A14

# BroadcastRadio
PRODUCT_PACKAGES += \
    vendor.visteon.hardware.TransportProtocol@V1-visteon-service \
    android.hardware.broadcastradio@V1-visteon-service 

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.broadcastradio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.broadcastradio.xml

###########
#QMAA flags starts
###########
#QMAA global flag for modular architecture
#true means QMAA is enabled for system
#false means QMAA is disabled for system

TARGET_USES_QMAA := true

#QMAA flag which is set to incorporate any generic dependencies
#required for the boot to UI flow in a QMAA enabled target.
#Set to false when all target level depenencies are met with
#actual full blown implementations.
TARGET_USES_QMAA_RECOMMENDED_BOOT_CONFIG := true

TARGET_USES_QMAA_OVERRIDE_ANDROID_CORE := true
TARGET_USES_QMAA_OVERRIDE_ANDROID_RECOVERY := true
TARGET_USES_QMAA_OVERRIDE_AUDIO   := true
TARGET_USES_QMAA_OVERRIDE_BIOMETRICS := true
TARGET_USES_QMAA_OVERRIDE_BLUETOOTH   := true
TARGET_USES_QMAA_OVERRIDE_CAMERA  := true
TARGET_USES_QMAA_OVERRIDE_CVP  := false
TARGET_USES_QMAA_OVERRIDE_DATA_NET := false
TARGET_USES_QMAA_OVERRIDE_DATA := false
TARGET_USES_QMAA_OVERRIDE_DIAG := false
TARGET_USES_QMAA_OVERRIDE_DISPLAY := true
TARGET_USES_QMAA_OVERRIDE_DPM  := false
TARGET_USES_QMAA_OVERRIDE_DRM  := true
TARGET_USES_QMAA_OVERRIDE_EID := false
TARGET_USES_QMAA_OVERRIDE_FASTCV  := true
TARGET_USES_QMAA_OVERRIDE_FASTRPC := false
TARGET_USES_QMAA_OVERRIDE_FM  := true
TARGET_USES_QMAA_OVERRIDE_FTM := false
TARGET_USES_QMAA_OVERRIDE_GFX := true
TARGET_USES_QMAA_OVERRIDE_GPS := true
TARGET_USES_QMAA_OVERRIDE_GP := true
TARGET_USES_QMAA_OVERRIDE_GPT := false
TARGET_USES_QMAA_OVERRIDE_KERNEL_TESTS_INTERNAL := false
TARGET_USES_QMAA_OVERRIDE_KMGK := true
TARGET_USES_QMAA_OVERRIDE_MSMIRQBALANCE := true
TARGET_USES_QMAA_OVERRIDE_OPENVX  := true
TARGET_USES_QMAA_OVERRIDE_PERF := true
TARGET_USES_QMAA_OVERRIDE_REMOTE_EFS := false
TARGET_USES_QMAA_OVERRIDE_RPMB := true
TARGET_USES_QMAA_OVERRIDE_SCVE  := false
TARGET_USES_QMAA_OVERRIDE_SECUREMSM_TESTS := true
TARGET_USES_QMAA_OVERRIDE_SENSORS := true
TARGET_USES_QMAA_OVERRIDE_SMCINVOKE := false
TARGET_USES_QMAA_OVERRIDE_SOTER := false
TARGET_USES_QMAA_OVERRIDE_SPCOM_UTEST := false
TARGET_USES_QMAA_OVERRIDE_SYNX := false
TARGET_USES_QMAA_OVERRIDE_TFTP := false
TARGET_USES_QMAA_OVERRIDE_USB := true
TARGET_USES_QMAA_OVERRIDE_VIBRATOR := false
TARGET_USES_QMAA_OVERRIDE_VIDEO   := true
TARGET_USES_QMAA_OVERRIDE_VPP := false
TARGET_USES_QMAA_OVERRIDE_WFD     := true
TARGET_USES_QMAA_OVERRIDE_WLAN    := true

TARGET_ENABLE_QSEECOM := false
#Full QMAA HAL List
QMAA_HAL_LIST := audio video camera display sensors gps

ifeq ($(TARGET_USES_QMAA), true)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.confqmaa=true
endif

###########
#QMAA flags ends

# Sensor conf files
PRODUCT_COPY_FILES += \
    device/qcom/msmnile_gvmq/sensors/hals.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors/hals.conf \
    frameworks/native/data/etc/android.hardware.sensor.hifi_sensors.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.hifi_sensors.xml

#Gnss
PRODUCT_PACKAGES += \
	android.hardware.gnss-service.example \

#telnetd
PRODUCT_PACKAGES += visteon.telnetd.service

#busybox_prebuilt
PRODUCT_PACKAGES += busybox_static_telnetd


#Initial bringup flags

#Default vendor image configuration
ifeq ($(ENABLE_VENDOR_IMAGE),)
ENABLE_VENDOR_IMAGE := false
endif

TARGET_KERNEL_VERSION := 6.1
TARGET_HAS_GENERIC_KERNEL_HEADERS := true

#Enable llvm support for kernel
KERNEL_LLVM_SUPPORT := true

#Enable sd-llvm suppport for kernel
KERNEL_SD_LLVM_SUPPORT := false

# default is nosdcard, S/W button enabled in resource
PRODUCT_CHARACTERISTICS := nosdcard

BOARD_FRP_PARTITION_NAME := frp

#Android EGL implementation
PRODUCT_PACKAGES += libGLES_android

#iperf2.0.9 integration
PRODUCT_PACKAGES += iperf2.0.9

# diag-router
ifeq ($(strip $(TARGET_BUILD_VARIANT)),user)
    TARGET_HAS_DIAG_ROUTER := false
else
    TARGET_HAS_DIAG_ROUTER := true
endif

# Memtrack HAL deprecated. Replaced with AIDL for target-level 6.
ENABLE_MEMTRACK_AIDL_HAL := true

-include $(QCPATH)/common/config/qtic-config.mk

PRODUCT_BOOT_JARS += tcmiface

ifneq ($(TARGET_NO_TELEPHONY), true)
 PRODUCT_BOOT_JARS += telephony-ext
 PRODUCT_PACKAGES += telephony-ext
endif

TARGET_DISABLE_DASH := true
TARGET_DISABLE_QTI_VPP := false

ifneq ($(TARGET_DISABLE_DASH), true)
    PRODUCT_BOOT_JARS += qcmediaplayer
endif

ifeq ($(TARGET_NO_QTI_WFD),)
    PRODUCT_BOOT_JARS += WfdCommon
endif

# Ethernet configuration file
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml

# Video codec configuration files
ifeq ($(TARGET_ENABLE_QC_AV_ENHANCEMENTS), true)
PRODUCT_COPY_FILES += device/qcom/msmnile/media_profiles.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_vendor.xml

PRODUCT_COPY_FILES += device/qcom/msmnile/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml
PRODUCT_COPY_FILES += device/qcom/msmnile/media_codecs_vendor.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_vendor.xml

PRODUCT_COPY_FILES += device/qcom/msmnile/media_codecs_vendor_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_vendor_audio.xml

PRODUCT_COPY_FILES += device/qcom/msmnile/media_codecs_performance.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_performance.xml
endif #TARGET_ENABLE_QC_AV_ENHANCEMENTS

#PRODUCT_COPY_FILES += hardware/qcom/media/conf_files/msmnile/system_properties.xml:$(TARGET_COPY_OUT_VENDOR)/etc/system_properties.xml

PRODUCT_PACKAGES += android.hardware.media.omx@1.0-impl

#Audio DLKM
AUDIO_DLKM := audio_apr.ko
AUDIO_DLKM += audio_snd_event.ko
AUDIO_DLKM += audio_q6_notifier.ko
AUDIO_DLKM += audio_adsp_loader.ko
AUDIO_DLKM += audio_q6.ko
AUDIO_DLKM += audio_platform.ko
AUDIO_DLKM += audio_hdmi.ko
AUDIO_DLKM += audio_stub.ko
AUDIO_DLKM += audio_native.ko
AUDIO_DLKM += audio_machine_msmnile.ko
PRODUCT_PACKAGES += $(AUDIO_DLKM)

# U-BRINGUP disable BT dlkm
# Bluetooth DLKM
#BT_DLKM := btpower.ko
#PRODUCT_PACKAGES += $(BT_DLKM)

PCIE_DLKM := pci_msm_drv
PRODUCT_PACKAGES += $(PCIE_DLKM)

# HS-I2S DLKM
#PRODUCT_PACKAGES += hsi2s.ko
# HS-I2S test app
PRODUCT_PACKAGES += hsi2s_test

#gptp app and daemon
PRODUCT_PACKAGES += gptp \
    libgptp.so \
    libgptp_test

#eavb fe lib and app
PRODUCT_PACKAGES += libeavbfe \
    eavbfe_test

PRODUCT_PACKAGES += fs_config_files

#A/B related packages
#PRODUCT_PACKAGES += update_engine \
#    update_engine_client \
#    update_verifier \
#    android.hardware.boot-service.qti.recovery \
#    android.hardware.boot-service.qti \

ifeq ($(TARGET_SINGLE_TREE), true)
PRODUCT_PACKAGES += android.hardware.boot@1.0-impl \
                    android.hardware.boot@1.0-service \
                    update_engine_sideload
endif
# bootctrl property
PRODUCT_VENDOR_PROPERTIES += \
    ro.vendor.bootctrl.enable=true

PRODUCT_PACKAGES += fstab.postinstall \
                    cppreopts.sh \
                    preloads_copy.sh \
                    cppreopts.rc

PRODUCT_HOST_PACKAGES += \
	brillo_update_payload

#Healthd packages
PRODUCT_PACKAGES += \
    libhealthd.msm

# MTMD enablement
PRODUCT_COPY_FILES += \
    device/qcom/msmnile_gvmq/input-port-associations.xml:$(TARGET_COPY_OUT_VENDOR)/etc/input-port-associations.xml \
    device/qcom/msmnile_gvmq/display_settings.xml:$(TARGET_COPY_OUT_VENDOR)/etc/display_settings.xml

DEVICE_MANIFEST_FILE := device/qcom/msmnile_gvmq/manifest.xml
DEVICE_MATRIX_FILE   := device/qcom/common/compatibility_matrix.xml
DEVICE_FRAMEWORK_MANIFEST_FILE := device/qcom/msmnile_gvmq/framework_manifest.xml

ifeq ($(TARGET_SINGLE_TREE), true)
  DEVICE_FRAMEWORK_MANIFEST_FILE := device/qcom/qssi_au/framework_manifest.xml
endif
DEVICE_FRAMEWORK_COMPATIBILITY_MATRIX_FILE := vendor/qcom/opensource/core-utils/vendor_framework_compatibility_matrix.xml

# Enable Scoped Storage related
$(call inherit-product, $(SRC_TARGET_DIR)/product/emulated_storage.mk)

# Display/Graphics
PRODUCT_PACKAGES += \
    android.hardware.broadcastradio@1.0-impl

# MSM IRQ Balancer configuration file
#PRODUCT_COPY_FILES += device/qcom/msmnile/msm_irqbalance.conf:$(TARGET_COPY_OUT_VENDOR)/etc/msm_irqbalance.conf

# MIDI feature
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml

# U-BRINGUP disable pro audio
# Pro Audio feature
#PRODUCT_COPY_FILES += \
#   frameworks/native/data/etc/android.hardware.audio.pro.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.audio.pro.xml

#Copy unsupported features list
#PRODUCT_COPY_FILES += \
#    device/qcom/msmnile_gvmq/msmnile_gvmq_excluded_features.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/msmnile_gvmq_excluded_features.xml


# Kernel modules install path
KERNEL_MODULES_INSTALL := dlkm
KERNEL_MODULES_OUT := out/target/product/msmnile_gvmq/$(KERNEL_MODULES_INSTALL)/lib/modules

#FEATURE_OPENGLES_EXTENSION_PACK support string config file
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.opengles.aep.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.opengles.aep.xml

#Enable full treble flag
PRODUCT_FULL_TREBLE_OVERRIDE := true
PRODUCT_VENDOR_MOVE_ENABLED := true
PRODUCT_COMPATIBLE_PROPERTY_OVERRIDE := true

#Enable vndk-sp Libraries
PRODUCT_PACKAGES += vndk_package
PRODUCT_PACKAGES += fstab.gen4.qcom

DEVICE_PACKAGE_OVERLAYS += device/qcom/msmnile_gvmq/overlay

# Enable flag to support slow devices
TARGET_PRESIL_SLOW_BOARD := true

ENABLE_VENDOR_RIL_SERVICE := true

#----------------------------------------------------------------------
# wlan specific
#----------------------------------------------------------------------
ifeq ($(strip $(BOARD_HAS_QCOM_WLAN)),true)
# Multiple chips
TARGET_WLAN_CHIP := qca6390 qca6490 qcn7605 qca6174
include device/qcom/wlan/msmnile_au/wlan.mk
endif

TARGET_MOUNT_POINTS_SYMLINKS := false


PRODUCT_PROPERTY_OVERRIDES += vendor.usb.diag_mdm.inst.name=diag_mdm2

# enable audio hidl hal 5.0
PRODUCT_PACKAGES += \
    android.hardware.audio@5.0 \
    android.hardware.audio.common@5.0 \
    android.hardware.audio.common@5.0-util \
    android.hardware.audio@5.0-impl \
    android.hardware.audio.effect@5.0 \
    android.hardware.audio.effect@5.0-impl

#Boot control HAL test app
PRODUCT_PACKAGES_DEBUG += bootctl
PRODUCT_PACKAGES += \
    libbt-vendor \
    android.hardware.bluetooth@1.0-service \
    android.hardware.bluetooth@1.0-impl \

PRODUCT_PACKAGES += \
   update_engine_sideload

#PRODUCT_PACKAGES += android.hardware.automotive.audiocontrol@1.0-service

PRODUCT_PACKAGES += android.hardware.health-service.example \
                    android.hardware.dumpstate-service.example

PRODUCT_PACKAGES += qcar-gsi.avbpubkey

#add vndservicemanager
PRODUCT_PACKAGES += vndservicemanager


#routingmanagerd
PRODUCT_PACKAGES += \
   routingmanagerd
PRODUCT_COPY_FILES += \
   external/vsomeip/config/vsomeip.json:/vendor/etc/vsomeip.json

#add neuralnetworks
PRODUCT_PACKAGES += android.hardware.neuralnetworks@1.0.vendor \
                    android.hardware.neuralnetworks@1.1.vendor \
                    android.hardware.neuralnetworks@1.2.vendor \
                    android.hardware.neuralnetworks@1.3.vendor

PRODUCT_ENFORCE_RRO_TARGETS := framework-res

#add libnbaio for avenhancement
PRODUCT_PACKAGES += libnbaio

PRODUCT_PRODUCT_PROPERTIES += persist.adb.tcp.port=5555

ifeq ($(TARGET_SINGLE_TREE), true)
  # Context hub HAL
  PRODUCT_PACKAGES += \
    android.hardware.contexthub@1.0-impl.generic \
    android.hardware.contexthub@1.0-service

  # system prop for enabling QFS (QTI Fingerprint Solution)
  PRODUCT_PROPERTY_OVERRIDES += \
    persist.vendor.qfp=true

  PRODUCT_SYSTEM_PROPERTIES += \
    persist.device_config.runtime_native_boot.iorap_perfetto_enable=true

  PRODUCT_SYSTEM_PROPERTIES += ro.android.car.audio.enableaudiopatch=true

  # USB default HAL
  #PRODUCT_PACKAGES += \
    android.hardware.usb@1.0-service

  #PASR HAL and APP
  #PRODUCT_PACKAGES += \
  # vendor.qti.power.pasrmanager@1.0-service \
  #  vendor.qti.power.pasrmanager@1.0-impl \
  #  pasrservice

  # CAN utils
  PRODUCT_PACKAGES += candump \
                    cansend \
                    bcmserver \
                    can-calc-bit-timing \
                    canbusload \
                    canfdtest \
                    cangen \
                    cangw \
                    canlogserver \
                    canplayer \
                    cansniffer \
                    isotpdump \
                    isotprecv \
                    isotpsend \
                    isotpserver \
                    isotptun \
                    log2asc \
                    log2long \
                    slcan_attach \
                    slcand \
                    slcanpty

  # copy system_ext specific whitelisted libraries to system_ext/etc
  PRODUCT_COPY_FILES += \
    device/qcom/qssi_au/public.libraries.system_ext-qti.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/public.libraries-qti.txt

  PRODUCT_PACKAGES += android.frameworks.automotive.display@1.0-service

  TARGET_USES_MKE2FS := true

  PRODUCT_PROPERTY_OVERRIDES += \
    ro.crypto.volume.filenames_mode = "aes-256-cts" \
    ro.crypto.allow_encrypt_override = true

endif

# Set network mode to (T/L/G/W/1X/EVDO, T/L/G/W/1X/EVDO) for 7+7 mode device on DSDS mode
PRODUCT_VENDOR_PROPERTIES += ro.telephony.default_network=22,22 \
                            ro.radio.noril=true

# system props for the cne module
PRODUCT_VENDOR_PROPERTIES += persist.vendor.cne.feature=1

#system props for the MM modules
PRODUCT_VENDOR_PROPERTIES += media.stagefright.enable-player=true \
                            media.stagefright.enable-http=true \
                            media.stagefright.enable-aac=true \
                            media.stagefright.enable-qcp=true \
                            media.stagefright.enable-fma2dp=true \
                            media.stagefright.enable-scan=true \

# system props for the data modules
PRODUCT_VENDOR_PROPERTIES += ro.vendor.use_data_netmgrd=true \
                            persist.vendor.data.mode=concurrent

# system prop for opengles version
# 196608 is decimal for 0x30000 to report version 3
# 196609 is decimal for 0x30001 to report version 3.1
# 196610 is decimal for 0x30002 to report version 3.2
PRODUCT_VENDOR_PROPERTIES += ro.opengles.version=196610

# system prop to turn on CdmaLTEPhone always
PRODUCT_VENDOR_PROPERTIES += telephony.lteOnCdmaDevice=1

BOARD_HAVE_QCOM_BLE_AUDIO := true

#system prop for wipower support
PRODUCT_VENDOR_PROPERTIES += ro.bluetooth.emb_wp_mode=false \
                            ro.bluetooth.wipower=false

PRODUCT_VENDOR_PROPERTIES += persist.vendor.service.bt.a2dp.sink=true \
                            persist.vendor.btstack.enable.splita2dp=false \
                            persist.vendor.service.bdroid.sibs=false \
                            persist.bt.clock_boottime_alarm=false

# system prop for Hardware type Automotive
PRODUCT_VENDOR_PROPERTIES += ro.hardware.type=automotive

PRODUCT_VENDOR_PROPERTIES += ro.hardware.sensors=msmnile.asm_auto

# snapdragon value add features
PRODUCT_VENDOR_PROPERTIES += ro.qc.sdk.audio.ssr=false

# fluencetype can be "fluence" or "fluencepro" or "none"
PRODUCT_VENDOR_PROPERTIES += ro.qc.sdk.audio.fluencetype=none \
                            persist.audio.fluence.voicecall=true \
                            persist.audio.fluence.voicerec=false \
                            persist.audio.fluence.speaker=true

# system prop for RmNet Data
PRODUCT_VENDOR_PROPERTIES += persist.rmnet.data.enable=true \
                            persist.data.wda.enable=true \
                            persist.data.df.dl_mode=5 \
                            persist.data.df.ul_mode=5 \
                            persist.data.df.agg.dl_pkt=10 \
                            persist.data.df.agg.dl_size=4096 \
                            persist.data.df.mux_count=8 \
                            persist.data.df.iwlan_mux=9 \
                            persist.data.df.dev_name=rmnet_usb0

# property to enable user to access Google WFD settings
PRODUCT_VENDOR_PROPERTIES += persist.debug.wfd.enable=1

# property to choose between virtual/external wfd display
PRODUCT_VENDOR_PROPERTIES += persist.sys.wfd.virtual=0

# enable tunnel encoding for amrwb
PRODUCT_VENDOR_PROPERTIES += tunnel.audio.encode = true

#Buffer size in kbytes for compress offload playback
PRODUCT_VENDOR_PROPERTIES += audio.offload.buffer.size.kb=32

# Enable offload audio video playback by default
PRODUCT_VENDOR_PROPERTIES += av.offload.enable=true

# Enable voice path for PCM VoIP by default
PRODUCT_VENDOR_PROPERTIES += use.voice.path.for.pcm.voip=true

# system prop for NFC DT
PRODUCT_VENDOR_PROPERTIES += ro.nfc.port=I2C

# Enable dsp gapless mode by default
PRODUCT_VENDOR_PROPERTIES += audio.offload.gapless.enabled=true

# initialize QCA1530 detection
PRODUCT_VENDOR_PROPERTIES += sys.qca1530=detect

# Enable stm events
PRODUCT_VENDOR_PROPERTIES += persist.debug.coresight.config=stm-events

# hwui properties
PRODUCT_VENDOR_PROPERTIES += ro.hwui.texture_cache_size=72 \
                            ro.hwui.layer_cache_size=48 \
                            ro.hwui.r_buffer_cache_size=8 \
                            ro.hwui.path_cache_size=32 \
                            ro.hwui.gradient_cache_size=1 \
                            ro.hwui.drop_shadow_cache_size=6 \
                            ro.hwui.texture_cache_flushrate=0.4 \
                            ro.hwui.text_small_cache_width=1024 \
                            ro.hwui.text_small_cache_height=1024 \
                            ro.hwui.text_large_cache_width=2048 \
                            ro.hwui.text_large_cache_height=1024 \

PRODUCT_VENDOR_PROPERTIES += config.disable_rtt=true

#Bringup properties
PRODUCT_VENDOR_PROPERTIES += persist.sys.force_sw_gles=1 \
                            persist.vendor.radio.atfwd.start=true \
                            ro.kernel.qemu.gles=0 \
                            qemu.hw.mainkeys=0

#Increase cached app limit
PRODUCT_VENDOR_PROPERTIES += ro.vendor.qti.sys.fw.bg_apps_limit=60

# Enable ZRAM
PRODUCT_VENDOR_PROPERTIES += ro.vendor.qti.config.zram=true

#IOP properties
PRODUCT_VENDOR_PROPERTIES += vendor.iop.enable_uxe=1 \
                            vendor.perf.iop_v3.enable=true

# Property to enable perf boosts from System Server
PRODUCT_VENDOR_PROPERTIES += vendor.perf.gestureflingboost.enable=true

#Enable ULMK properties
PRODUCT_VENDOR_PROPERTIES += ro.lmk.kill_heaviest_task=true \
                            ro.lmk.kill_timeout_ms=15 \
                            ro.lmk.use_minfree_levels=true \
                            ro.lmk.enhance_batch_kill=true \
                            ro.lmk.enable_adaptive_lmk=true \
                            ro.lmk.vmpressure_file_min=80640 \

#Property to enable scroll pre-obtain view
PRODUCT_VENDOR_PROPERTIES += ro.vendor.scroll.preobtain.enable=true

#Display mirroring
PRODUCT_VENDOR_PROPERTIES += vendor.display.builtin_mirroring=true

#Display hwcId allocation
PRODUCT_VENDOR_PROPERTIES += vendor.display.builtin_baseid_and_size=5,3 \
                            vendor.display.pluggable_baseid_and_size=1,4 \
                            vendor.display.virtual_baseid_and_size=8,1 \

#USB unsupported
PRODUCT_COPY_FILES += \
                      device/qcom/msmnile_gvmq/usb_err_event.sh:system/bin/usb_err_event.sh
# Disable boot animation
PRODUCT_PROPERTY_OVERRIDES += debug.sf.nobootanimation=1

# Enable CPMS for LPM
PRODUCT_VENDOR_PROPERTIES += persist.vendor.car.lpm=true

# default wifi country code
PRODUCT_VENDOR_PROPERTIES += ro.boot.wificountrycode=us

# The property "persist.bluetooth.enablenewavrcp" is introduced in AOSP.
# See commit e63f6d6bda16bd94d43537fc5db754a103c6a757
# (1) If the property is set as true, it indicates that AVRCP(TG) is enabled.
# (2) If the property is set as false, it indicates that AVRCP(CT) is enabled.
# In Fluoride Bluetooth stack, the default value for the property is true. This is valid with Mobile SP.
# However in Automotive SP, AVRCP(CT) is enabled in Car UI.
# So the property should be set as false.
PRODUCT_VENDOR_PROPERTIES += persist.bluetooth.enablenewavrcp=false

# Add gsi avb keys
PRODUCT_PACKAGES += qcar-gsi.avbpubkey

ifeq ($(TARGET_SINGLE_TREE), true)
  # Include mainline components and QSSI whitelist
  ifeq (true,$(call math_gt_or_eq,$(SHIPPING_API_LEVEL),29))
    $(call inherit-product, device/qcom/qssi_au/qssi_au_whitelist.mk)
    PRODUCT_ARTIFACT_PATH_REQUIREMENT_IGNORE_PATHS := /system/system_ext/
    PRODUCT_ENFORCE_ARTIFACT_PATH_REQUIREMENTS := false
  endif

  PRODUCT_PACKAGES += vendor.qti.qesdsys
endif
PRODUCT_PACKAGES += vendor.visteon.cluster \
    vendor.visteon.cluster-service \

BOARD_SEPOLICY_DIRS += vendor/visteon/hardware/interfaces/clusterhal/sepolicy
# --------------------------------------- nfore bluetooth block start ------------------

$(call inherit-product, vendor/nforetek/nf3805/bluetooth/bluetooth_product_package.mk)
$(call inherit-product, vendor/nforetek/nf2702/bluetooth_nf2702.mk)

# Connectivity - Bluetooth
USES_NF3805_BT := true

ifeq ($(USES_NF3805_BT),true)
# NFORE TECHNOLOGY NF3805 BLUETOOTH
BOARD_HAVE_BLUETOOTH_NF3805 := true
endif

# --------------------------------------- nfore bluetooth block end --------------------

# --------------------------------------- nfore Wi-Fi block start ----------------------

$(call inherit-product, vendor/nforetek/nf3805/wlan/wlan_product_package.mk)

# Connectivity - Wi-Fi
USES_NF3805_WLAN := true

# NFORE TECHNOLOGY NF3805 Wi-Fi
ifeq ($(USES_NF3805_WLAN),true)
BOARD_WLAN_DEVICE := MediaTek
endif

# --------------------------------------- nfore Wi-Fi block end -----------------------
# Wifi regulatory
PRODUCT_COPY_FILES += \
    vendor/visteon/wifi_product/libs/config/db.txt:vendor/firmware/db.txt \
    external/wireless-regdb/regulatory.db:$(TARGET_COPY_OUT_VENDOR)/firmware/regulatory.db \
    external/wireless-regdb/regulatory.db.p7s:$(TARGET_COPY_OUT_VENDOR)/firmware/regulatory.db.p7s

PRODUCT_PROPERTY_OVERRIDES += ro.vendor.wifi.sap.interface=ap0
PRODUCT_PROPERTY_OVERRIDES += wlan.driver.status=ok

# WiFi HAL
PRODUCT_PACKAGES += \
        android.hardware.wifi-service \
        wificond \
        lib_driver_cmd_mt66xx.a \
        libwifi-hal-mt66xx \
        wifitesttool


#ExtAudioHal
PRODUCT_PACKAGES += vendor.visteon.hardware.extaudiohal \
		    vendor.visteon.hardware.extaudiohal@V1-service \
#Audiofmq
PRODUCT_PACKAGES += vendor.visteon.hardware.audiofmq \
    vendor.visteon.hardware.audiofmq@1.0-service \

#Persistency
PRODUCT_PACKAGES += vendor.visteon.automotive.persistency-service \
		    MC_PersistencyAidlTest \
		    PersistencyAidlTest \

#VREngineInterface, Alexa_release, Alexa_lmApp
PRODUCT_PACKAGES += \
    VREngineInterface \
    Alexa_release \
    Alexa_lmApp

PRODUCT_PACKAGES += ipset
#Tunnel Proxy Service
PRODUCT_PACKAGES += TunnelProxyService
PRODUCT_PACKAGES += libtun2http


# privapp permissions for Alexa and VREngineInterface
PRODUCT_COPY_FILES += \
    vendor/visteon/packages/apps/Alexa/privapp-permissions-com.amazon.alexa.auto.app.xml:system/etc/permissions/privapp-permissions-com.amazon.alexa.auto.app.xml \
    vendor/visteon/packages/apps/VREngineInterface/privapp-permissions-com.visteon.vrengineinterface.xml:system/etc/permissions/privapp-permissions-com.visteon.vrengineinterface.xml


###################################################################################
# This is the End of target.mk file.
# Now, Pickup other split product.mk files:
###################################################################################
# TODO: Relocate the system product.mk files pickup into qssi lunch, once it is up.
$(call inherit-product-if-exists, vendor/qcom/defs/product-defs/system/*.mk)
$(call inherit-product-if-exists, vendor/qcom/defs/product-defs/vendor/*.mk)
###################################################################################

$(call inherit-product, $(LOCAL_PATH)/rui.mk)

# Dolby Specific
$(call inherit-product, $(LOCAL_PATH)/dolby.mk)
$(call inherit-product, vendor/dolby/device/common/external.mk)
