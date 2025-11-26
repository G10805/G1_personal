#LOCAL_PATH:= $(call my-dir)
LOCAL_PATH:= $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/WNR_Intf

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/wnr_on/wnr_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/wnr_on/wnr_config.txt \
    $(LOCAL_PATH)/configs/wnr_on/wnr_params.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/wnr_on/wnr_params.txt \
    $(LOCAL_PATH)/configs/wnr_off/wnr_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/wnr_off/wnr_config.txt \
    $(LOCAL_PATH)/configs/wnr_off/wnr_params.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/wnr_off/wnr_params.txt \
