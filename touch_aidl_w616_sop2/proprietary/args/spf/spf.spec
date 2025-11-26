Summary: Signal Processing Framework (SPF)
Name: spf
Version: 1.0
Release: r0
License: Qualcomm-Technologies-Inc.-Proprietary
URL: http://support.cdmatech.com
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake libtool gcc-g++ systemd-rpm-macros

%description
This is the Signal Processing Framework header files spec.

%package -n spf-devel
Summary: Signal Processing Framework (SPF) - Development files
License: Qualcomm-Technologies-Inc.-Proprietary

%description -n spf-devel
This is the Signal Processing Framework header files spec.
This package contains symbolic links, header files
and related items necessary for software development.

%global debug_package %{nil}
%global __os_install_post %{nil}

%prep
%autosetup -n %{name}-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE -DCMAKE_RULE_MESSAGES:BOOL=OFF -DCMAKE_VERBOSE_MAKEFILE:BOOL=$VER -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir} -DCMAKE_INSTALL_LIBDIR=%{_libdir} -DCMAKE_INSTALL_BINDIR=%{_bindir} -DCMAKE_INSTALL_LICENSEDIR=%{_defaultlicensedir}
%cmake_build

%install
%cmake_install

%files
%license NOTICE

%files -n spf-devel
%{_includedir}/acd_api.h
%{_includedir}/amdb_api.h
%{_includedir}/apm_api.h
%{_includedir}/apm_api_version.h
%{_includedir}/apm_container_api.h
%{_includedir}/apm_graph_properties.h
%{_includedir}/apm_memmap_api.h
%{_includedir}/apm_module_api.h
%{_includedir}/apm_sub_graph_api.h
%{_includedir}/aptx_adaptive_encoder_api.h
%{_includedir}/aptx_adaptive_swb_decoder_api.h
%{_includedir}/aptx_adaptive_swb_encoder_api.h
%{_includedir}/aptx_classic_decoder_api.h
%{_includedir}/aptx_classic_encoder_api.h
%{_includedir}/aptx_hd_decoder_api.h
%{_includedir}/aptx_hd_encoder_api.h
%{_includedir}/ar_defs.h
%{_includedir}/ar_error_codes.h
%{_includedir}/ar_ids.h
%{_includedir}/audio_dam_buffer_api.h
%{_includedir}/audio_hw_clk_api.h
%{_includedir}/audio_hw_dma_api.h
%{_includedir}/audio_hw_lpm_api.h
%{_includedir}/codec_dma_api.h
%{_includedir}/common_enc_dec_api.h
%{_includedir}/contexts_api.h
%{_includedir}/cop_packetizer_cmn_api.h
%{_includedir}/cop_packetizer_v0_api.h
%{_includedir}/cop_v2_depacketizer_api.h
%{_includedir}/cop_v2_packetizer_api.h
%{_includedir}/data_logging_api.h
%{_includedir}/detection_cmn_api.h
%{_includedir}/display_port_api.h
%{_includedir}/dtmf_detection_api.h
%{_includedir}/fluence_ffv_common_calibration.h
%{_includedir}/gapless_api.h
%{_includedir}/gate_api.h
%{_includedir}/gpio_api.h
%{_includedir}/hw_core_api.h
%{_includedir}/hw_intf_cmn_api.h
%{_includedir}/i2s_api.h
%{_includedir}/imcl_fwk_intent_api.h
%{_includedir}/imcl_spm_intent_api.h
%{_includedir}/irm_api.h
%{_includedir}/lc3_decoder_api.h
%{_includedir}/lc3_encoder_api.h
%{_includedir}/ldac_encoder_api.h
%{_includedir}/lpass_core_api.h
%{_includedir}/mailbox_api.h
%{_includedir}/media_fmt_api.h
%{_includedir}/media_fmt_api_basic.h
%{_includedir}/media_fmt_api_ext.h
%{_includedir}/metadata_api.h
%{_includedir}/mfc_api.h
%{_includedir}/module_cmn_api.h
%{_includedir}/mux_demux_api.h
%{_includedir}/pcm_encoder_api.h
%{_includedir}/pcm_tdm_api.h
%{_includedir}/pop_suppressor_api.h
%{_includedir}/priority_sync_api.h
%{_includedir}/prm_api.h
%{_includedir}/rate_adapted_timer_api.h
%{_includedir}/rd_sh_mem_client_api.h
%{_includedir}/rd_sh_mem_ep_api.h
%{_includedir}/rtm_logging_api.h
%{_includedir}/rt_proxy_api.h
%{_includedir}/sh_mem_pull_push_mode_api.h
%{_includedir}/slimbus_api.h
%{_includedir}/smart_sync_api.h
%{_includedir}/spf_begin_pack.h
%{_includedir}/spf_begin_pragma.h
%{_includedir}/spf_end_pack.h
%{_includedir}/spf_end_pragma.h
%{_includedir}/splitter_api.h
%{_includedir}/spr_api.h
%{_includedir}/sp_dc_api.h
%{_includedir}/sp_rx.h
%{_includedir}/sp_vi.h
%{_includedir}/sync_api.h
%{_includedir}/test_module_api.h
%{_includedir}/trm_api.h
%{_includedir}/tty_cmn_api.h
%{_includedir}/usb_api.h
%{_includedir}/us_detect_api.h
%{_includedir}/vcpm_api.h
%{_includedir}/vocoder_cmn_api.h
%{_includedir}/wr_sh_mem_client_api.h
%{_includedir}/wr_sh_mem_ep_api.h
