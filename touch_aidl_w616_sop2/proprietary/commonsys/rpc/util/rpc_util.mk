# install rpc util lib
PRODUCT_PACKAGES += libqti_hal_rpc_util
PRODUCT_PACKAGES += libqti_log_util
PRODUCT_PACKAGES += libqti_message_sched_util
PRODUCT_PACKAGES += libqti_proto_util
PRODUCT_PACKAGES += libqti_rpc_common_util
PRODUCT_PACKAGES += libqti_someip_util
PRODUCT_PACKAGES += libqti_someip_util@1.1

# install bt hal rpc lib
PRODUCT_PACKAGES += libqti-bluetooth-message
PRODUCT_PACKAGES += libqti-bluetooth-rpc

# install bt hf audio rpc lib
PRODUCT_PACKAGES += libqti-bluetooth-handsfree-audio-message
PRODUCT_PACKAGES += libqti-bluetooth-handsfree-audio-rpc

# install wifi hal rpc lib
PRODUCT_PACKAGES += libqti-wifi-message
PRODUCT_PACKAGES += libqti-wifi-supplicant-message
PRODUCT_PACKAGES += libqti-wifi-hostapd-message
PRODUCT_PACKAGES += libqti-net-nlinterceptor-message

# install wifi vendor rpc lib
PRODUCT_PACKAGES += libqti-hostapd-vendor-message
PRODUCT_PACKAGES += libqti-supplicant-vendor-message

RPC_UTIL_PATH := $(PWD)/vendor/qcom/proprietary/commonsys/rpc/util
# run a2p script
ifeq ($(ENABLE_QTI_A2P), true)
A2P_SCRIPT := $(RPC_UTIL_PATH)/tool/aidl_2_proto/bin/a2p.sh
ifneq "$(wildcard $(A2P_SCRIPT))" ""
$(shell chmod 777 $(A2P_SCRIPT))
$(shell $(A2P_SCRIPT) $(PWD) $(VENDOR_AIDL_PATH))
endif
endif

# run apcc script
ifeq ($(ENABLE_QTI_APCC), true)
APCC_SCRIPT := $(RPC_UTIL_PATH)/tool/apcc/bin/apcc.sh
ifneq "$(wildcard $(APCC_SCRIPT))" ""
$(shell chmod 777 $(APCC_SCRIPT))
$(shell $(APCC_SCRIPT) $(PWD) $(VENDOR_AIDL_PATH))
endif
endif

# run rpc_class_gen script
ifeq ($(ENABLE_QTI_RPC_CLASS_GEN), true)
RPC_CLASS_GEN_SCRIPT := $(RPC_UTIL_PATH)/tool/rpc_class_gen/bin/rpc_class_gen.sh
ifneq "$(wildcard $(RPC_CLASS_GEN_SCRIPT))" ""
$(shell chmod 777 $(RPC_CLASS_GEN_SCRIPT))
$(shell $(RPC_CLASS_GEN_SCRIPT) $(PWD))
endif
endif
