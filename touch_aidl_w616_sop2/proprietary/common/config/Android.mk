LOCAL_PATH := $(my-dir)

#----------------------------------------------------------------------
# Copy public.libraries.txt to /system/vendor/etc
#----------------------------------------------------------------------
PUBLIC_LIBS_TARGET := $(TARGET_OUT_VENDOR_ETC)/public.libraries.txt
$(PUBLIC_LIBS_TARGET):
	@mkdir -p $(TARGET_OUT_VENDOR_ETC)
	-@cat device/qcom/$(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)/public.libraries.vendor.txt > $@
	-@if ["$(TARGET_SUPPORTS_ANDROID_WEAR)" == "true"] ; then \
		cat $(QC_PROP_ROOT)/common/config/public.libraries.wearable.txt >> $@; \
	else \
		cat $(QC_PROP_ROOT)/common/config/public.libraries.txt >> $@; \
	fi

ifneq ($(wildcard device/qcom/$(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)/public.libraries.vendor.txt),)
  $(PUBLIC_LIBS_TARGET): device/qcom/$(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)/public.libraries.vendor.txt
endif
ifeq ($(TARGET_SUPPORTS_ANDROID_WEAR),true)
  ifneq ($(wildcard $(QC_PROP_ROOT)/common/config/public.libraries.wearable.txt),)
    $(PUBLIC_LIBS_TARGET): $(QC_PROP_ROOT)/common/config/public.libraries.wearable.txt
  endif
else
  ifneq ($(wildcard $(QC_PROP_ROOT)/common/config/public.libraries.txt),)
    $(PUBLIC_LIBS_TARGET): $(QC_PROP_ROOT)/common/config/public.libraries.txt
  endif
endif

ALL_DEFAULT_INSTALLED_MODULES += $(PUBLIC_LIBS_TARGET)
#----------------------------------------------------------------------

#----------------------------------------------------------------------
# Copy public.libraries-qti.txt to /system/etc
#----------------------------------------------------------------------
PUBLIC_LIBS_QTI_TARGET := $(TARGET_OUT_ETC)/public.libraries-qti.txt
$(PUBLIC_LIBS_QTI_TARGET):
	-@cat device/qcom/$(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)/public.libraries-qti.txt > $@
	-@cat $(QC_PROP_ROOT)/common/config/public.libraries-qti.txt >> $@

ifneq ($(wildcard device/qcom/$(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)/public.libraries-qti.txt),)
  $(PUBLIC_LIBS_QTI_TARGET): device/qcom/$(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX)$(TARGET_BOARD_DERIVATIVE_SUFFIX)/public.libraries-qti.txt
endif
ifneq ($(wildcard $(QC_PROP_ROOT)/common/config/public.libraries-qti.txt),)
  $(PUBLIC_LIBS_QTI_TARGET): $(QC_PROP_ROOT)/common/config/public.libraries-qti.txt
endif

ALL_DEFAULT_INSTALLED_MODULES += $(PUBLIC_LIBS_QTI_TARGET)
#----------------------------------------------------------------------

ifeq ($(ENABLE_EXTRA_VENDOR_LIBS),true)
ifeq ($(BOARD_VNDK_VERSION),)

EXTRA_VENDOR_LIBRARIES := \
    vendor.qti.hardware.slmadapter@1.0 \
    vendor.qti.hardware.mwqemadapter@1.0 \
    com.qualcomm.qti.ant@1.0 \
    com.qualcomm.qti.bluetooth_audio@1.0 \
    com.qualcomm.qti.dpm.api@1.0 \
    com.qualcomm.qti.imscmservice@2.0 \
    com.qualcomm.qti.imscmservice@2.1 \
    vendor.qti.hardware.bluetooth_audio@2.0 \
    vendor.display.color@1.0 \
    vendor.display.config@1.0 \
    vendor.display.postproc@1.0 \
    vendor.qti.data.factory@1.0 \
    vendor.qti.data.factory@2.0 \
    vendor.qti.esepowermanager@1.0 \
    vendor.qti.gnss@1.0 \
    vendor.qti.hardware.alarm@1.0 \
    vendor.qti.hardware.btconfigstore@1.0 \
    vendor.qti.hardware.bluetooth_sar@1.0 \
    vendor.qti.hardware.camera.device@1.0 \
    vendor.qti.hardware.cryptfshw@1.0 \
    vendor.qti.hardware.data.dynamicdds@1.0 \
    vendor.qti.hardware.cacert@1.0 \
    vendor.qti.hardware.data.cne.internal.api@1.0 \
    vendor.qti.hardware.data.cne.internal.constants@1.0 \
    vendor.qti.hardware.data.cne.internal.server@1.0 \
    vendor.qti.hardware.data.connection@1.0 \
    vendor.qti.hardware.data.connection@1.1 \
    vendor.qti.hardware.data.latency@1.0 \
    vendor.qti.hardware.data.qmi@1.0 \
    vendor.qti.hardware.factory@1.0 \
    vendor.qti.hardware.factory@1.1 \
    vendor.qti.hardware.fingerprint@1.0 \
    vendor.qti.hardware.fm@1.0 \
    vendor.qti.hardware.iop@1.0 \
    vendor.qti.hardware.perf@1.0 \
    vendor.qti.hardware.qdutils_disp@1.0 \
    vendor.qti.hardware.qteeconnector@1.0 \
    vendor.qti.hardware.radio.am@1.0 \
    vendor.qti.hardware.radio.ims@1.0 \
    vendor.qti.hardware.radio.lpa@1.0 \
    vendor.qti.hardware.radio.qcrilhook@1.0 \
    vendor.qti.hardware.radio.qtiradio@1.0 \
    vendor.qti.hardware.radio.qtiradio@2.1 \
    vendor.qti.hardware.radio.qtiradio@2.2 \
    vendor.qti.hardware.radio.internal.deviceinfo@1.0 \
    vendor.qti.hardware.radio.uim@1.0 \
    vendor.qti.hardware.radio.uim_remote_client@1.0 \
    vendor.qti.hardware.radio.uim_remote_client@1.1 \
    vendor.qti.hardware.radio.uim_remote_server@1.0 \
    vendor.qti.hardware.sensorscalibrate@1.0 \
    vendor.qti.hardware.tui_comm@1.0 \
    vendor.qti.hardware.wifi.supplicant@2.0 \
    vendor.qti.hardware.wifi.supplicant@2.1 \
    vendor.qti.hardware.wigig.netperftuner@1.0 \
    vendor.qti.hardware.wigig.supptunnel@1.0 \
    vendor.qti.ims.callinfo@1.0 \
    vendor.qti.voiceprint@1.0 \
    vendor.display.color@1.1 \
    vendor.display.config@1.1 \
    vendor.qti.gnss@1.1 \
    vendor.qti.hardware.radio.ims@1.1 \
    vendor.qti.hardware.radio.uim@1.1 \
    vendor.qti.hardware.radio.uim@1.2 \
    vendor.qti.hardware.radio.atcmdfwd@1.0 \
    vendor.qti.hardware.vpp@1.1 \
    vendor.display.color@1.2 \
    vendor.display.config@1.2 \
    vendor.qti.gnss@1.2 \
    vendor.qti.hardware.radio.ims@1.2 \
    vendor.qti.hardware.radio.ims@1.3 \
    vendor.qti.hardware.radio.ims@1.4 \
    vendor.qti.hardware.radio.ims@1.5 \
    vendor.qti.gnss@2.0 \
    vendor.qti.hardware.perf@2.0 \
    vendor.qti.hardware.iop@2.0 \
    vendor.qti.hardware.radio.qtiradio@2.0 \
    vendor.qti.gnss@2.1 \
    vendor.qti.gnss@3.0 \
    vendor.display.config@1.3 \
    vendor.display.config@1.4 \
    vendor.display.config@1.5 \
    vendor.display.config@1.6 \
    vendor.display.config@1.7 \
    vendor.qti.ims.rcsconfig@1.0 \
    vendor.qti.imsrtpservice@2.0 \
    vendor.qti.imsrtpservice@2.1 \
    libvndfwk_detect_jni.qti \
    vendor.qti.latency@2.0 \
    libdrm
ifeq ($(call is-board-platform-in-list,msm8996),false)
EXTRA_VENDOR_LIBRARIES += vendor.qti.hardware.improvetouch.touchcompanion@1.0
endif

define define-extra-vendor-lib
include $$(CLEAR_VARS)
LOCAL_MODULE := $1.extra-vendor-lib-gen
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,SHARED_LIBRARIES,$1)/$1.so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := first
LOCAL_MODULE_TAGS := optional
LOCAL_INSTALLED_MODULE_STEM := $1.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_VENDOR_MODULE := true
include $$(BUILD_PREBUILT)

ifneq ($$(TARGET_2ND_ARCH),)
ifneq ($$(TARGET_TRANSLATE_2ND_ARCH),true)
include $$(CLEAR_VARS)
LOCAL_MODULE := $1.extra-vendor-lib-gen
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PREBUILT_MODULE_FILE := $$(call intermediates-dir-for,SHARED_LIBRARIES,$1,,,$(my_2nd_arch_prefix))/$1.so
LOCAL_STRIP_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := optional
LOCAL_INSTALLED_MODULE_STEM := $1.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_VENDOR_MODULE := true
include $$(BUILD_PREBUILT)
endif # TARGET_TRANSLATE_2ND_ARCH is not true
endif # TARGET_2ND_ARCH is not empty
endef

$(foreach lib,$(EXTRA_VENDOR_LIBRARIES),\
    $(eval $(call define-extra-vendor-lib,$(lib))))

include $(CLEAR_VARS)
LOCAL_MODULE := vendor-extra-libs
LOCAL_MODULE_TAGS := optional
LOCAL_REQUIRED_MODULES := $(addsuffix .extra-vendor-lib-gen,$(EXTRA_VENDOR_LIBRARIES))
include $(BUILD_PHONY_PACKAGE)
endif
endif
