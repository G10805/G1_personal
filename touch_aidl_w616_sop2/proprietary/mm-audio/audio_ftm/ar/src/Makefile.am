# Source Path

AM_CFLAGS =             -Wundef \
                        -Wstrict-prototypes \
                        -Wno-trigraphs \
                        -g -O0 \
                        -fno-inline \
                        -fno-short-enums \
                        -DMSM8960_ALSA \
                        -fpic \
                        -I../inc \
                        $(DIAG_CFLAGS) \
                        $(TINYALSA_CFLAGS) \
                        -I. \
                        -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/audio-kernel \
                        -I $(WORKSPACE)/system/core/include

#AM_CFLAGS := -I ${top_srcdir}/ar/inc
AM_CFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/audio-kernel
AM_CFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/agm
AM_CFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/
AM_CFLAGS += -I $(WORKSPACE)/system/core/include
AM_CFLAGS += -I $(WORKSPACE)/audio/mm-audio-headers/acdbdata/
AM_CFLAGS += -I $(WORKSPACE)/vendor/qcom/opensource/agm/service/inc/
AM_CFLAGS += -I $(WORKSPACE)/diag/include/


audftm_c_sources =      DALSYS_common.c \
                        audio_ftm_afe_loopback.c \
                        audio_ftm_driver_fwk.c \
                        audio_ftm_dtmf_basic_op.c \
                        audio_ftm_dtmf_tone_gen.c \
                        audio_ftm_hw_drv_ar.c \
                        audio_ftm_pcm_loopback.c \
                        audio_ftm_pcm_record.c \
                        audio_ftm_tone_play.c \
                        audio_ftm_util_fifo.c \
                        ftm_audio_dispatch_ar.c \
                        ftm_audio_main.c \
                        audio_ftm_diag_mem.c \
                        audio_ftm_pcm_play.c \
                        audio_ftm_ext_loopback.c \
                        fft.c

bin_PROGRAMS = mm_audio_ftm
mm_audio_ftm_SOURCES = $(audftm_c_sources) $(h_sources)
mm_audio_ftm_la_CC = @CC@
mm_audio_ftm_CFLAGS = $(AM_CFLAGS) -DTINYALSA_LIB -DUSE_GLIB @GLIB_CFLAGS@ -DQC_MODIFIED -DFEATURE_EXPORT_SND -DFEATURE_AUDIO_AGC -DFEATURE_VOC_PCM_INTERFACE -DFEATURE_VOICE_PLAYBACK -DFEATURE_VOICE_RECORD -DVERBOSE -D_DEBUG -DMSM8960_ALSA -DINT_MAX=0x7fffffff -include log/logd.h -D_Nonnull="" -DTINYALSA_LIB
mm_audio_ftm_LDADD = -lm -ltinyalsa -lcutils -llog $(DIAG_LIBS) $(GLIB_LIBS) -ldl
