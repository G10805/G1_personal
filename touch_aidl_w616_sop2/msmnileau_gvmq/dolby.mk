# Dolby specific
PRODUCT_RESTRICT_VENDOR_FILES := false
DOLBY_DCX_CODEC2 := true
PRODUCT_COPY_FILES += vendor/dolby/device/common/media_codecs_dolby_c2_dcx.xml:vendor/etc/media_codecs_dolby_c2_dcx.xml
PRODUCT_COPY_FILES += vendor/dolby/device/aosp_car/media_codec_40/dolby_dcx_config.xml:vendor/etc/dolby_dcx_config_40.xml
PRODUCT_COPY_FILES += vendor/dolby/device/aosp_car/media_codec_512/dolby_dcx_config.xml:vendor/etc/dolby_dcx_config_512.xml

DOLBY_DCX_CODEC2_PCM := true
# if your DCX output configuration is 7.1.4, 7.1.2 or 7.1
PRODUCT_COPY_FILES += vendor/dolby/device/common/media_codecs_dolby_c2_dcx_pcm_decoder_71x.xml:vendor/etc/media_codecs_dolby_c2_dcx_pcm_decoder_71x.xml
# if your DCX output configuration is 5.1.4, 5.1.2, 5.1 or 4.0
PRODUCT_COPY_FILES += vendor/dolby/device/common/media_codecs_dolby_c2_dcx_pcm_decoder_51x.xml:vendor/etc/media_codecs_dolby_c2_dcx_pcm_decoder_51x.xml