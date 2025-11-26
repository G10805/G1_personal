#LOCAL_PATH:= $(call my-dir)
LOCAL_PATH:= $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/ANS

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/ans_on/ans_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/ans_on/ans_config.txt \
    $(LOCAL_PATH)/configs/ans_off/ans_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/ans_off/ans_config.txt \
