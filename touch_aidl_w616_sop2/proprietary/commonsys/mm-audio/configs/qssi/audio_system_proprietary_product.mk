$(call inherit-product-if-exists, $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/AdaptiveEQ/config.mk)
$(call inherit-product-if-exists, $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/AGC/config.mk)
$(call inherit-product-if-exists, $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/ANS/config.mk)
$(call inherit-product-if-exists, $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/Compressor/config.mk)
$(call inherit-product-if-exists, $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/HDR/config.mk)
$(call inherit-product-if-exists, $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/HPF/config.mk)
$(call inherit-product-if-exists, $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/Limiter/config.mk)
$(call inherit-product-if-exists, $(QCPATH)/commonsys/mm-audio/audio-systems/microeffects/WNR_Intf/config.mk)

MM_AUDIO := libAGC_recpp
MM_AUDIO += libCompressor_recpp
MM_AUDIO += libAdaptiveEQ_recpp
MM_AUDIO += libHDR_recpp
MM_AUDIO += libHPFilter_recpp
MM_AUDIO += libLimiter_recpp
MM_AUDIO += librecpp_intf
MM_AUDIO += libWNR
MM_AUDIO += libWNR_intf
MM_AUDIO += libANS_recpp

PRODUCT_PACKAGES += $(MM_AUDIO)