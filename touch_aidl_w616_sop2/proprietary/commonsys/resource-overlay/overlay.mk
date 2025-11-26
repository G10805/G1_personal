#include common overlays
ifneq ($(strip $(TARGET_BOARD_AUTO)),true)
PRODUCT_PACKAGES += \
    FrameworksResCommon_Sys \
    CarrierConfigResCommon_Sys \
    CellBroadcastReceiverResCommon_Sys \
    SystemUIResCommon_Sys \
    TelecommResCommon_Sys \
    TelephonyResCommon_Sys \
    FrameworksResCommonQva_Sys \
    SettingsResCommon_Sys \
    WifiResCommon_Sys
else
PRODUCT_PACKAGES += \
    FrameworksResAutoCommon_Sys \
    SystemUIResAutoCommon_Sys \
    WifiResAutoCommon_Sys
endif

