#LOCAL_PATH:= $(call my-dir)
LOCAL_PATH:= $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/HPF

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/main_inv_ls/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/main_inv_ls/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/main_inv_pt/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/main_inv_pt/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/main_ls/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/main_ls/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/main_pt/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/main_pt/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/none_inv_ls/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/none_inv_ls/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/none_inv_pt/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/none_inv_pt/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/none_ls/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/none_ls/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/none_pt/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/none_pt/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/selfie_inv_ls/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/selfie_inv_ls/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/selfie_inv_pt/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/selfie_inv_pt/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/selfie_ls/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/selfie_ls/RPP_BiquadHPF_config.txt \
    $(LOCAL_PATH)/configs/selfie_pt/RPP_BiquadHPF_config.txt:$(TARGET_COPY_OUT_SYSTEM_EXT)/etc/hdr_config/selfie_pt/RPP_BiquadHPF_config.txt \
