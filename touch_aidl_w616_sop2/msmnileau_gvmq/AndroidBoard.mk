LOCAL_PATH := $(call my-dir)

#----------------------------------------------------------------------
# Compile (L)ittle (K)ernel bootloader and the nandwrite utility
#----------------------------------------------------------------------
ifneq ($(strip $(TARGET_NO_BOOTLOADER)),true)

# Compile
include bootable/bootloader/edk2/AndroidBoot.mk

$(INSTALLED_BOOTLOADER_MODULE): $(TARGET_EMMC_BOOTLOADER) | $(ACP)
	$(transform-prebuilt-to-target)
$(BUILT_TARGET_FILES_PACKAGE): $(INSTALLED_BOOTLOADER_MODULE)

droidcore: $(INSTALLED_BOOTLOADER_MODULE)
endif

# Create firmware folder for graphics
$(shell mkdir -p $(TARGET_OUT_VENDOR)/firmware/)

ifeq ($(TARGET_KERNEL_SOURCE),)
     TARGET_KERNEL_SOURCE := kernel
endif

DTC := $(HOST_OUT_EXECUTABLES)/dtc$(HOST_EXECUTABLE_SUFFIX)
UFDT_APPLY_OVERLAY := $(HOST_OUT_EXECUTABLES)/ufdt_apply_overlay$(HOST_EXECUTABLE_SUFFIX)

# Record current directory to handle invoking sub-makes with -C
# TODO(b/112561200) creating the TEMP_TOP variable undoes the intent of
# removing ANDROID_BUILD_TOP, but it allows the build to succeed.
TEMP_TOP=$(shell pwd)
TARGET_KERNEL_MAKE_ENV := DTC_EXT=$(TEMP_TOP)/$(DTC)
TARGET_KERNEL_MAKE_ENV += DTC_OVERLAY_TEST_EXT=$(TEMP_TOP)/$(UFDT_APPLY_OVERLAY)
TARGET_KERNEL_MAKE_ENV += CONFIG_BUILD_ARM64_DT_OVERLAY=y
TARGET_KERNEL_MAKE_ENV += HOSTCC=$(TEMP_TOP)/$(SOONG_LLVM_PREBUILTS_PATH)/clang
TARGET_KERNEL_MAKE_ENV += HOSTAR=$(TEMP_TOP)/prebuilts/gcc/linux-x86/host/x86_64-linux-glibc2.17-4.8/bin/x86_64-linux-ar
TARGET_KERNEL_MAKE_ENV += HOSTLD=$(TEMP_TOP)/prebuilts/gcc/linux-x86/host/x86_64-linux-glibc2.17-4.8/bin/x86_64-linux-ld
TARGET_KERNEL_MAKE_ENV += HOSTCFLAGS="-I$$(pwd)/kernel/msm-4.14/include/uapi -I/usr/include -I/usr/include/x86_64-linux-gnu -L/usr/lib -L/usr/lib/x86_64-linux-gnu -fuse-ld=lld"
TARGET_KERNEL_MAKE_ENV += HOSTLDFLAGS="-L/usr/lib -L/usr/lib/x86_64-linux-gnu -fuse-ld=lld"

#include $(TARGET_KERNEL_SOURCE)/AndroidKernel.mk
#include device/qcom/kernelscripts/kernel_definitions.mk
$(TARGET_PREBUILT_KERNEL): $(DTC) $(UFDT_APPLY_OVERLAY)

$(INSTALLED_KERNEL_TARGET): $(TARGET_PREBUILT_KERNEL) | $(ACP)
	$(transform-prebuilt-to-target)

#----------------------------------------------------------------------
# Copy additional target-specific files
#----------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE       := vold.fstab
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := init.target.rc
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)/init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := gpio-keys.kl
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_PATH  := $(TARGET_OUT_KEYLAYOUT)
include $(BUILD_PREBUILT)

ifeq ($(strip $(BOARD_DYNAMIC_PARTITION_ENABLE)),true)
  include $(CLEAR_VARS)
  LOCAL_MODULE       := fstab.qcom
  LOCAL_MODULE_TAGS  := optional
  LOCAL_MODULE_CLASS := ETC
  ifeq ($(ENABLE_AB), true)
    ifeq (true,$(call math_gt_or_eq,$(PRODUCT_SHIPPING_API_LEVEL),34))
        LOCAL_SRC_FILES := gen4_fstab_metadata_f2fs/fstab_AB_dynamic_partition_variant.gen4.qti
    else
        LOCAL_SRC_FILES := fstab_AB_dynamic_partition_variant.qti
    endif
  else
    ifeq (true,$(call math_gt_or_eq,$(PRODUCT_SHIPPING_API_LEVEL),34))
        LOCAL_SRC_FILES := gen4_fstab_metadata_f2fs/fstab_non_AB_dynamic_partition_variant.gen4.qti
    else
        LOCAL_SRC_FILES := fstab_non_AB_dynamic_partition_variant.qti
    endif
  endif #ENABLE_AB
  LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)
  include $(BUILD_PREBUILT)

  # Add gen4 AB fstab
  include $(CLEAR_VARS)
  LOCAL_MODULE       := fstab.gen4.qcom
  LOCAL_MODULE_TAGS  := optional
  LOCAL_MODULE_CLASS := ETC
  ifeq ($(ENABLE_AB), true)
    ifeq (true,$(call math_gt_or_eq,$(PRODUCT_SHIPPING_API_LEVEL),34))
        LOCAL_SRC_FILES := gen4_fstab_metadata_f2fs/fstab_AB_dynamic_partition_variant.gen4.qti
    else
        LOCAL_SRC_FILES := fstab_AB_dynamic_partition_variant.gen4.qti
    endif
  else
    ifeq (true,$(call math_gt_or_eq,$(PRODUCT_SHIPPING_API_LEVEL),34))
        LOCAL_SRC_FILES := gen4_fstab_metadata_f2fs/fstab_non_AB_dynamic_partition_variant.gen4.qti
    else
        LOCAL_SRC_FILES := fstab_non_AB_dynamic_partition_variant.gen4.qti
    endif
  endif #ENABLE_AB
  LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)
  include $(BUILD_PREBUILT)
  include $(CLEAR_VARS)
  LOCAL_MODULE       := fstab.gen3.ufs.qcom
  LOCAL_MODULE_TAGS  := optional
  LOCAL_MODULE_CLASS := ETC
  ifeq ($(ENABLE_AB), true)
    LOCAL_SRC_FILES := 6155_ufs/fstab_AB_dynamic_partition_variant.qti
  else
    LOCAL_SRC_FILES := 6155_ufs/fstab_non_AB_dynamic_partition_variant.qti
  endif #ENABLE_AB
  LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)
  include $(BUILD_PREBUILT)
  include $(CLEAR_VARS)
  LOCAL_MODULE       := fstab.gen3.emmc.qcom
  LOCAL_MODULE_TAGS  := optional
  LOCAL_MODULE_CLASS := ETC
  ifeq ($(ENABLE_AB), true)
    LOCAL_SRC_FILES := 6155_emmc/fstab_AB_dynamic_partition_variant.qti
  else
    LOCAL_SRC_FILES := 6155_emmc/fstab_non_AB_dynamic_partition_variant.qti
  endif #ENABLE_AB
  LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)
  include $(BUILD_PREBUILT)
else
  include $(CLEAR_VARS)
  LOCAL_MODULE       := fstab.qcom
  LOCAL_MODULE_CLASS := ETC
  LOCAL_SRC_FILES    := $(LOCAL_MODULE)
  LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)
  ifeq ($(ENABLE_VENDOR_IMAGE), true)
    LOCAL_POST_INSTALL_CMD := echo $(VENDOR_FSTAB_ENTRY) >> $(LOCAL_MODULE_PATH)/$(LOCAL_MODULE)
  endif
  include $(BUILD_PREBUILT)

  # Add gen4 non AB fstab
  include $(CLEAR_VARS)
  LOCAL_MODULE       := fstab.gen4.qcom
  LOCAL_MODULE_CLASS := ETC
  ifeq (true,$(call math_gt_or_eq,$(PRODUCT_SHIPPING_API_LEVEL),34))
      LOCAL_SRC_FILES    := gen4_fstab_metadata_f2fs/$(LOCAL_MODULE)
  else
      LOCAL_SRC_FILES    := $(LOCAL_MODULE)
  endif
  LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)
  ifeq ($(ENABLE_VENDOR_IMAGE), true)
    LOCAL_POST_INSTALL_CMD := echo $(VENDOR_FSTAB_ENTRY) >> $(LOCAL_MODULE_PATH)/$(LOCAL_MODULE)
  endif
  include $(BUILD_PREBUILT)
endif ##BOARD_DYNAMIC_PARTITION_ENABLE

#Add gsi key.
include $(CLEAR_VARS)
LOCAL_MODULE := qcar-gsi.avbpubkey
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0
LOCAL_LICENSE_CONDITIONS := notice
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := ../../../test/vts-testcase/security/avb/data/qcar-gsi.avbpubkey
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)/first_stage_ramdisk/avb
include $(BUILD_PREBUILT)

include device/qcom/vendor-common/MergeConfig.mk

#----------------------------------------------------------------------
# Radio image
#----------------------------------------------------------------------
ifeq ($(ADD_RADIO_FILES), true)
radio_dir := $(LOCAL_PATH)/radio
RADIO_FILES := $(shell cd $(radio_dir) ; ls)
$(foreach f, $(RADIO_FILES), \
	$(call add-radio-file,radio/$(f)))
endif

#----------------------------------------------------------------------
# extra images
#----------------------------------------------------------------------
include device/qcom/common/generate_extra_images.mk

include vendor/qcom/opensource/core-utils/build/AndroidBoardCommon.mk

#----------------------------------------------------------------------
# override default make with prebuilt make path (if any)
#----------------------------------------------------------------------
ifneq (, $(wildcard $(shell pwd)/prebuilts/build-tools/linux-x86/bin/make))
    MAKE := $(shell pwd)/prebuilts/build-tools/linux-x86/bin/$(MAKE)
endif
