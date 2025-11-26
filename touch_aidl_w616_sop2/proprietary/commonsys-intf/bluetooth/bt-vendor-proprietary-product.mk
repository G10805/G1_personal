#ANT
PRODUCT_PACKAGES += com.dsi.ant@1.0.vendor
PRODUCT_PACKAGES += com.dsi.ant@1.0-impl

#BT - #Visteon commented
#ifeq ($(BOARD_HAVE_QTI_BT_LAZY_SERVICE),true)
#PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-service-qti-lazy
#PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-service-qti-lazy.rc
#else
#PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-service-qti
#PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-service-qti.rc
#endif

#Visteon commented
#ifeq ($(TARGET_USE_QTI_BT_AIDL), true)
#PRODUCT_PACKAGES += android.hardware.bluetooth-service-qti
#PRODUCT_PACKAGES += android.hardware.bluetooth-service-qti.rc
#PRODUCT_PACKAGES += android.hardware.bluetooth-impl-qti
#endif

ifeq ($(TARGET_USE_QTI_BT_HAL_RPC),true)
ifneq ($(TARGET_USE_QTI_BT_RPC_NEW),true)
PRODUCT_PACKAGES += libqti_bt_hal_rpc_impl
PRODUCT_PACKAGES += libqti_bt_hci_rpc
ifeq ($(TARGET_ENABLE_QTI_BT_HCI_PROTO), true)
PRODUCT_PACKAGES += libqti_bt_hci_message
endif
ifeq ($(TARGET_USE_QTI_BT_HAL_RPC_SERVER), true)
PRODUCT_PACKAGES += bt_hal_service
endif
endif
ifeq ($(TARGET_USE_QTI_BT_HF_AUDIO_RPC), true)
PRODUCT_PACKAGES += VtsBtHfAudio
endif
endif

#Visteon commented PRODUCT_PACKAGES += android.hardware.bluetooth@1.0-impl-qti
#Visteon commented PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_audio@2.0-impl
#Visteon commented PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_audio@2.1-impl
PRODUCT_PACKAGES += android.hardware.bluetooth.a2dp@2.0-impl
PRODUCT_PACKAGES += android.hardware.bluetooth.audio-impl
#Visteon commented PRODUCT_PACKAGES += com.qualcomm.qti.bluetooth_audio@1.0.vendor
#Visteon commented PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_audio@2.0.vendor
#Visteon commented PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_audio@2.1.vendor
PRODUCT_PACKAGES += btaudio_offload_if
PRODUCT_PACKAGES += audio.bluetooth.default
PRODUCT_PACKAGES += audio.bluetooth_qti.default
PRODUCT_PACKAGES += libbluetooth_audio_session_qti
PRODUCT_PACKAGES += libbluetooth_audio_session_qti_2_1
PRODUCT_PACKAGES += libbluetooth_audio_session
PRODUCT_PACKAGES += libbt-hidlclient
PRODUCT_PACKAGES += vendor.qti.hardware.btconfigstore@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.btconfigstore@1.0-impl
PRODUCT_PACKAGES += vendor.qti.hardware.btconfigstore@2.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.btconfigstore@2.0-impl
PRODUCT_PACKAGES += libbtnv
PRODUCT_PACKAGES_DEBUG += btconfig

#BTSAR
ifeq ($(TARGET_USE_QTI_BT_SAR),true)
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_sar@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_sar@1.0-impl
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_sar@1.1.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_sar@1.1-impl
endif #TARGET_USE_QTI_BT_SAR

#BTDUN
ifeq ($(TARGET_USE_BT_DUN),true)
ifeq ($(TARGET_HAS_DUN_HIDL_FEATURE),true)
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0-impl
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0-service
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0-service.rc
PRODUCT_PACKAGES += vendor.qti.hardware.bluetooth_dun@1.0-service.disable.rc
endif #TARGET_HAS_DUN_HIDL_FEATURE
endif #TARGET_USE_BT_DUN

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml


#FM
PRODUCT_PACKAGES += fmconfig
PRODUCT_PACKAGES_DEBUG += fmfactorytest
PRODUCT_PACKAGES_DEBUG += fmfactorytestserver
PRODUCT_PACKAGES += fm_qsoc_patches
PRODUCT_PACKAGES += ftm_fm_lib
PRODUCT_PACKAGES += vendor.qti.hardware.fm@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.fm@1.0-impl
PRODUCT_PACKAGES_DEBUG += hal_ss_test_manual
PRODUCT_PACKAGES += init.qti.fm.sh


#WIPOWER
ifeq ($(BOARD_USES_WIPOWER),true)
PRODUCT_PACKAGES += vendor.qti.hardware.wipower@1.0_vendor
PRODUCT_PACKAGES += vendor.qti.hardware.wipower@1.0-impl
endif #BOARD_USES_WIPOWER

ifeq ($(TARGET_BOARD_TYPE),auto)
ifeq ($(BOARD_HAVE_QCOM_BLE_AUDIO),true)
# Enable BLE Audio profile in AUTO
PRODUCT_PROPERTY_OVERRIDES +=  \
    bluetooth.profile.bap.broadcast.assist.enabled=true \
    bluetooth.profile.bap.broadcast.source.enabled=true \
    bluetooth.profile.bap.unicast.client.enabled=true \
    bluetooth.profile.csip.set_coordinator.enabled=true \
    bluetooth.profile.vcp.controller.enabled=true \
    bluetooth.profile.mcp.server.enabled=true \
    bluetooth.profile.ccp.server.enabled=true
endif
endif

#ANT/BT/FM/WIPOWER PROPERTIES

ifeq ($(TARGET_BOARD_PLATFORM),gen5) # gen5 specific defines
ifeq ($(TARGET_BOARD_TYPE),auto)
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=rome
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=rome
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),gen4) # gen4 specific defines
ifeq ($(TARGET_BOARD_TYPE),auto)
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=rome
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=rome
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),msmnile) # msmnile specific defines
ifeq ($(TARGET_BOARD_TYPE),auto)
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=rome
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=rome
else
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptive
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=true
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=false
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),sdm845) # SDM845 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=true
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=false
endif


ifeq ($(TARGET_BOARD_PLATFORM),sdmshrike)  # Poipu/8195 specific defines
ifeq ($(TARGET_BOARD_TYPE),auto)
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=rome
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=rome
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),sm6150)  # Talos/sm6150 specific defines
ifeq ($(TARGET_BOARD_TYPE),auto)
PRODUCT_PROPERTY_OVERRIDES += vendor.qcom.bluetooth.soc=rome
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=rome
else
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=true
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=false
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),sdm710)  # sdm710/Warlock specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=true
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),kona)  # kona/sm8250 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=hastings
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptiver2
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#A2dp Multicast support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=true
#AptX Adaptive R2.1 support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aptxadaptiver2_1_support=false
#HearingAid support
PRODUCT_PROPERTY_OVERRIDES += persist.sys.fflag.override.settings_bluetooth_hearing_aid=true
endif

ifeq ($(TARGET_BOARD_PLATFORM), lahaina) # lahaina specific defines
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptiver2
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#A2dp Multicast support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=true
#AptX Adaptive R2.1 support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aptxadaptiver2_1_support=true
#HearingAid support
PRODUCT_PROPERTY_OVERRIDES += persist.sys.fflag.override.settings_bluetooth_hearing_aid=true
endif

ifeq ($(TARGET_BOARD_PLATFORM), taro) # waipio specific defines
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptiver2
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#A2dp Multicast support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=true
#AptX Adaptive R2.1 support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aptxadaptiver2_1_support=true
#HearingAid support
PRODUCT_PROPERTY_OVERRIDES += persist.sys.fflag.override.settings_bluetooth_hearing_aid=true
endif

ifeq ($(TARGET_BOARD_PLATFORM), kalama) # kailua specific defines
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptiver2
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#A2dp Multicast support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=true
#AptX Adaptive R2.1 support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aptxadaptiver2_1_support=true
#HearingAid support
PRODUCT_PROPERTY_OVERRIDES += persist.sys.fflag.override.settings_bluetooth_hearing_aid=true
endif

ifeq ($(TARGET_BOARD_PLATFORM), pineapple) # lanai specific defines
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptiver2
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#A2dp Multicast support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=true
#AptX Adaptive R2.1 support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aptxadaptiver2_1_support=true
#HearingAid support
PRODUCT_PROPERTY_OVERRIDES += persist.sys.fflag.override.settings_bluetooth_hearing_aid=true
endif

ifeq ($(TARGET_BOARD_PLATFORM), trinket) # trinket specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=true
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM), lito) # lito specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptiver2
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=true
#AptX Adaptive R2.1 support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aptxadaptiver2_1_support=false
#HearingAid support
PRODUCT_PROPERTY_OVERRIDES += persist.sys.fflag.override.settings_bluetooth_hearing_aid=true
endif

ifeq ($(TARGET_BOARD_PLATFORM), bengal) # bengal specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM), holi) # holi specific defines
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),atoll) # atoll specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxtws-aptxhd-aac-ldac-aptxadaptive
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#aac frame control support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled=true
#TWS+ state support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.twsp_state.enabled=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
#AAC VBR support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.aac_vbr_ctl.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),sdm660)  # sdm660
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8998) # MSM8998 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=cherokee
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=true
#a2dp offload capability
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.a2dp_offload_cap=sbc-aptx-aptxhd-aac-ldac
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
#Scrambling support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.scram.enabled=true
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8996) # MSM8996 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=rome
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
# Set this true as ROME which is programmed
# as embedded wipower mode by default
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8937) # msm8937 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=pronto
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8953) # MSM8953 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=pronto
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8909) # MSM8909 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=pronto
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8952) # MSM8952 specific defines
#Bluetooth SOC type
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.soc=pronto
#split a2dp support
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.qcom.bluetooth.enable.splita2dp=false
#Embedded wipower mode
PRODUCT_PROPERTY_OVERRIDES += ro.vendor.bluetooth.wipower=false
endif

#
# BT HAL mode property
#   (1) default:  BT HAL for QCA chip is enabled
#   (2) bluecore: BT HAL for QCSR BlueCore chip (e.g. CSR8311) is enabled
#   (3) rpc:      BT HAL RPC is enabled
#
PRODUCT_PROPERTY_OVERRIDES += persist.vendor.bluetooth.hal_mode=default
