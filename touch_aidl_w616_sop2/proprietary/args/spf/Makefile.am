pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = spf.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wall -Werror
if BUILDSYSTEM_OPENWRT
AM_CFLAGS += -I ./api
AM_CFLAGS += -I ./api/apm
AM_CFLAGS += -I ./api/ar_utils
AM_CFLAGS += -I ./api/modules
AM_CFLAGS += -I ./api/vcpm

spf_sources = ./api/apm/apm_api.h \
                ./api/apm/apm_api_version.h \
                ./api/apm/apm_container_api.h \
                ./api/apm/apm_graph_properties.h \
                ./api/apm/apm_memmap_api.h \
                ./api/apm/apm_module_api.h \
                ./api/apm/apm_prv_api.h \
                ./api/apm/apm_sub_graph_api.h \
                ./api/ar_utils/spf_begin_pack.h \
                ./api/ar_utils/spf_begin_pragma.h \
                ./api/ar_utils/spf_end_pack.h \
                ./api/ar_utils/spf_end_pragma.h \
                ./api/ar_utils/ar_error_codes.h \
                ./api/ar_utils/ar_guids.h \
                ./api/ar_utils/ar_ids.h \
                ./api/ar_utils/ar_msg.h \
                ./api/ar_utils/ar_msg_base.h \
                ./api/ar_utils/ar_defs.h \
                ./api/ar_utils/ar_std.h \
                ./api/modules/amdb_api.h \
                ./api/modules/audio_dam_buffer_api.h \
                ./api/modules/audio_hw_clk_api.h \
                ./api/modules/audio_hw_dma_api.h \
                ./api/modules/audio_hw_lpm_api.h \
                ./api/modules/codec_dma_api.h \
                ./api/modules/common_enc_dec_api.h \
                ./api/modules/data_logging_api.h \
                ./api/modules/detection_cmn_api.h \
                ./api/modules/display_port_api.h \
                ./api/modules/dtmf_detection_api.h \
                ./api/modules/ec_sync_api.h \
                ./api/modules/hw_intf_cmn_api.h \
                ./api/modules/i2s_api.h \
                ./api/modules/mailbox_api.h \
                ./api/modules/imcl_fwk_intent_api.h \
                ./api/modules/lpass_core_api.h \
                ./api/modules/media_fmt_api.h \
                ./api/modules/metadata_api.h \
                ./api/modules/module_cmn_api.h \
                ./api/modules/pcm_tdm_api.h \
                ./api/modules/prm_api.h \
                ./api/modules/rate_adapted_timer_api.h \
                ./api/modules/splitter_api.h \
                ./api/modules/rd_sh_mem_client_api.h \
                ./api/modules/rd_sh_mem_ep_api.h \
                ./api/modules/rtm_logging_api.h \
                ./api/modules/sh_mem_pull_push_mode_api.h \
                ./api/modules/slimbus_api.h \
                ./api/modules/spr_api.h \
                ./api/modules/sync_api.h \
                ./api/modules/test_module_api.h \
                ./api/modules/usb_api.h \
                ./api/modules/wr_sh_mem_client_api.h \
                ./api/modules/wr_sh_mem_ep_api.h \
                ./api/modules/vocoder_cmn_api.h \
                ./api/modules/gapless_api.h \
                ./api/modules/hw_core_api.h \
                ./api/modules/trm_api.h \
                ./api/modules/tty_cmn_api.h \
                ./api/modules/rt_proxy_api.h \
                ./api/vcpm/vcpm_api.h

else
AM_CFLAGS += -I ${top_srcdir}/api
AM_CFLAGS += -I ${top_srcdir}/api/asps
AM_CFLAGS += -I ${top_srcdir}/api/apm
AM_CFLAGS += -I ${top_srcdir}/api/ar_utils
AM_CFLAGS += -I ${top_srcdir}/api/modules
AM_CFLAGS += -I ${top_srcdir}/api/vcpm

spf_sources = ${top_srcdir}/api/apm/apm_api.h \
                ${top_srcdir}/api/apm/apm_api_version.h \
                ${top_srcdir}/api/apm/apm_container_api.h \
                ${top_srcdir}/api/apm/apm_graph_properties.h \
                ${top_srcdir}/api/apm/apm_memmap_api.h \
                ${top_srcdir}/api/apm/apm_module_api.h \
                ${top_srcdir}/api/apm/apm_sub_graph_api.h \
                ${top_srcdir}/api/ar_utils/spf_begin_pack.h \
                ${top_srcdir}/api/ar_utils/spf_begin_pragma.h \
                ${top_srcdir}/api/ar_utils/spf_end_pack.h \
                ${top_srcdir}/api/ar_utils/spf_end_pragma.h \
                ${top_srcdir}/api/ar_utils/ar_error_codes.h \
                ${top_srcdir}/api/ar_utils/ar_guids.h \
                ${top_srcdir}/api/ar_utils/ar_ids.h \
                ${top_srcdir}/api/ar_utils/ar_defs.h \
                ${top_srcdir}/api/modules/amdb_api.h \
                ${top_srcdir}/api/modules/aptx_adaptive_swb_decoder_api.h \
                ${top_srcdir}/api/modules/aptx_adaptive_swb_encoder_api.h \
                ${top_srcdir}/api/modules/aptx_classic_decoder_api.h \
                ${top_srcdir}/api/modules/aptx_hd_decoder_api.h \
                ${top_srcdir}/api/modules/aptx_hd_encoder_api.h \
                ${top_srcdir}/api/modules/audio_dam_buffer_api.h \
                ${top_srcdir}/api/modules/audio_hw_clk_api.h \
                ${top_srcdir}/api/modules/audio_hw_dma_api.h \
                ${top_srcdir}/api/modules/audio_hw_lpm_api.h \
                ${top_srcdir}/api/modules/codec_dma_api.h \
                ${top_srcdir}/api/modules/common_enc_dec_api.h \
                ${top_srcdir}/api/modules/data_logging_api.h \
                ${top_srcdir}/api/modules/detection_cmn_api.h \
                ${top_srcdir}/api/modules/display_port_api.h \
                ${top_srcdir}/api/modules/dtmf_detection_api.h \
                ${top_srcdir}/api/modules/gate_api.h \
                ${top_srcdir}/api/modules/hw_intf_cmn_api.h \
                ${top_srcdir}/api/modules/i2s_api.h \
                ${top_srcdir}/api/modules/imcl_fwk_intent_api.h \
                ${top_srcdir}/api/modules/irm_api.h \
                ${top_srcdir}/api/modules/ldac_encoder_api.h \
                ${top_srcdir}/api/modules/lpass_core_api.h \
                ${top_srcdir}/api/modules/mailbox_api.h \
                ${top_srcdir}/api/modules/media_fmt_api.h \
                ${top_srcdir}/api/modules/metadata_api.h \
                ${top_srcdir}/api/modules/module_cmn_api.h \
                ${top_srcdir}/api/modules/mux_demux_api.h \
                ${top_srcdir}/api/modules/pcm_tdm_api.h \
                ${top_srcdir}/api/modules/priority_sync_api.h \
                ${top_srcdir}/api/modules/prm_api.h \
                ${top_srcdir}/api/modules/rate_adapted_timer_api.h \
                ${top_srcdir}/api/modules/rd_sh_mem_client_api.h \
                ${top_srcdir}/api/modules/rd_sh_mem_ep_api.h \
                ${top_srcdir}/api/modules/rtm_logging_api.h \
                ${top_srcdir}/api/modules/sh_mem_pull_push_mode_api.h \
                ${top_srcdir}/api/modules/slimbus_api.h \
                ${top_srcdir}/api/modules/smart_sync_api.h \
                ${top_srcdir}/api/modules/splitter_api.h \
                ${top_srcdir}/api/modules/spr_api.h \
                ${top_srcdir}/api/modules/sync_api.h \
                ${top_srcdir}/api/modules/test_module_api.h \
                ${top_srcdir}/api/modules/usb_api.h \
                ${top_srcdir}/api/modules/wr_sh_mem_client_api.h \
                ${top_srcdir}/api/modules/wr_sh_mem_ep_api.h \
                ${top_srcdir}/api/modules/vocoder_cmn_api.h \
                ${top_srcdir}/api/modules/gapless_api.h \
                ${top_srcdir}/api/modules/hw_core_api.h \
                ${top_srcdir}/api/modules/trm_api.h \
                ${top_srcdir}/api/modules/tty_cmn_api.h \
                ${top_srcdir}/api/modules/rt_proxy_api.h \
                ${top_srcdir}/api/vcpm/vcpm_api.h \
                ${top_srcdir}/api/modules/media_fmt_api_basic.h \
                ${top_srcdir}/api/modules/media_fmt_api_ext.h \
                ${top_srcdir}/api/modules/cop_packetizer_cmn_api.h \
                ${top_srcdir}/api/modules/imcl_spm_intent_api.h \
                ${top_srcdir}/api/modules/aptx_classic_encoder_api.h \
                ${top_srcdir}/api/modules/aptx_adaptive_encoder_api.h \
                ${top_srcdir}/api/modules/sp_dc_api.h \
                ${top_srcdir}/api/modules/sp_rx.h \
                ${top_srcdir}/api/modules/sp_vi.h \
                ${top_srcdir}/api/modules/cop_v2_depacketizer_api.h \
                ${top_srcdir}/api/modules/cop_v2_packetizer_api.h \
                ${top_srcdir}/api/modules/us_detect_api.h \
                ${top_srcdir}/api/modules/lc3_encoder_api.h \
                ${top_srcdir}/api/modules/acd_api.h \
                ${top_srcdir}/api/modules/lc3_decoder_api.h \
                ${top_srcdir}/api/modules/contexts_api.h \
                ${top_srcdir}/api/modules/pcm_encoder_api.h \
                ${top_srcdir}/api/modules/cop_packetizer_v0_api.h \
                ${top_srcdir}/api/modules/pop_suppressor_api.h \
                ${top_srcdir}/api/modules/fluence_ffv_common_calibration.h

asps_sources =  ${top_srcdir}/api/asps/asps_ads_usecase_api.h \
                ${top_srcdir}/api/asps/asps_upd_usecase_api.h \
                ${top_srcdir}/api/asps/asps_acd_usecase_api.h \
                ${top_srcdir}/api/asps/asps_acm_api.h

endif

spf_includedir = $(includedir)
spf_include_HEADERS = $(spf_sources)

asps_includedir = $(includedir)/spf/asps
asps_include_HEADERS = $(asps_sources)

#LIBRARY for LE apps
lib_LTLIBRARIES = libspf.la
libspf_la_CC = @CC@
libspf_la_SOURCES = $(spf_sources)
libspf_la_CFLAGS = $(AM_CFLAGS)
libspf_la_LDFLAGS = -shared -avoid-version
