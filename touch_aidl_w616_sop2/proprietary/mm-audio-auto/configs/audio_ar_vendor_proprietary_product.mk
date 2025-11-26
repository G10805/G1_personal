# MM_AUDIO_AR_PROP for proprietary libraries.
# MM_AUDIO_AUDIOLITE_PROP for audiolite proprietary product
ifneq ($(AUDIO_USE_STUB_HAL), true)
ifneq (,$(filter $(TARGET_BOARD_PLATFORM)$(TARGET_BOARD_SUFFIX), msmnile_gvmq gen4_gvm msmnile_au gen4_au))

MM_AUDIO_AR_PROP := libacdbloadersocketclient
MM_AUDIO_AR_PROP += libar-acdb
MM_AUDIO_AR_PROP += libar-gpr
MM_AUDIO_AR_PROP += libadm_ar
ifeq ($(ENABLE_HYP), true)
MM_AUDIO_AR_PROP += libar-gsl_fe
else
MM_AUDIO_AR_PROP += libar-gsl
endif
MM_AUDIO_AR_PROP += liblx-ar_util
MM_AUDIO_AR_PROP += liblx-osal
MM_AUDIO_AR_PROP += hw_ep_info.xml
MM_AUDIO_AR_PROP += resourcemanager_sa8155_adp_star.xml
MM_AUDIO_AR_PROP += resourcemanager_sa8255_adp_star.xml
ifeq ($(ENABLE_HYP), true)
MM_AUDIO_AR_PROP += resourcemanager_8155.xml
MM_AUDIO_AR_PROP += resourcemanager_6155.xml
MM_AUDIO_AR_PROP += resourcemanager_gvmauto_8155.xml
MM_AUDIO_AR_PROP += resourcemanager_gvmauto_6155.xml
MM_AUDIO_AR_PROP += resourcemanager_gvmauto8295_adp_star.xml
MM_AUDIO_AR_PROP += resourcemanager_gvmauto8255_adp_star.xml
MM_AUDIO_AR_PROP += resourcemanager_gvmauto7255_adp_star.xml
ifeq ($(TARGET_USES_GY), true)
MM_AUDIO_AR_PROP += resourcemanager_VIOSND.xml
MM_AUDIO_AR_PROP += usecaseKvManager-VIOSND.xml
endif
endif
MM_AUDIO_AR_PROP += usecaseKvManager.xml

PRODUCT_PACKAGES += $(MM_AUDIO_AR_PROP)

endif

else # AUDIO_USE_STUB_HAL is true

ifeq ($(TARGET_USES_AUDIOLITE), true)
MM_AUDIO_AUDIOLITE_PROP += audiolite
MM_AUDIO_AUDIOLITE_PROP += ipcc_shmem_ping_test.sh

PRODUCT_PACKAGES += $(MM_AUDIO_AUDIOLITE_PROP)
endif

endif # end AUDIO_USE_STUB_HAL
