/*
 * Copyright (c) 2019, 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * kvh2xml.h
 *
 * This header file defines the keys to be used in acdbdata.
 *
 */

 #ifndef __KVH_2_XML_H__
 #define __KVH_2_XML_H__

typedef enum {
    PLATFORM_LA = 1,    /**< @h2xmle_name {LA} */
    PLATFORM_LE = 2,    /**< @h2xmle_name {LE} */
}platforms;
/**
    @h2xml_platforms{PLATFORM_LA,PLATFORM_LE}
*/

enum AllKeyIds{
        STREAMRX            = 0xA1000000,    /**< @h2xmle_name{StreamRX} */
        DEVICERX            = 0xA2000000,    /**< @h2xmle_name{DeviceRX} */
        DEVICETX            = 0xA3000000,    /**< @h2xmle_name{DeviceTX} */
        VOLUME              = 0xA4000000,    /**< @h2xmle_name{Volume} */
        SAMPLINGRATE        = 0xA5000000,    /**< @h2xmle_name{SamplingRate} */
        BITWIDTH            = 0xA6000000,    /**< @h2xmle_name{BitWidth} */
        PAUSE               = 0xA7000000,    /**< @h2xmle_name{Pause} */
        MUTE                = 0xA8000000,    /**< @h2xmle_name{Mute} */
        CHANNELS            = 0xA9000000,    /**< @h2xmle_name{Channels} */
        FLUENCE             = 0xAA000000,    /**< @h2xmle_name{Fluence} */
        INSTANCE            = 0xAB000000,    /**< @h2xmle_name{Instance} */
        DEVICEPP_RX         = 0xAC000000,    /**< @h2xmle_name{DevicePP_Rx} */
        DEVICEPP_TX         = 0xAD000000,    /**< @h2xmle_name{DevicePP_Tx} */
        MEDIA_FMT_ID        = 0xAE000000,    /**< @h2xmle_name{MediaFmtID} */
        STREAMPP_RX         = 0xAF000000,    /**< @h2xmle_name{StreamPP_RX} */
        STREAMPP_TX         = 0xB0000000,    /**< @h2xmle_name{StreamPP_TX} */
        STREAMTX            = 0xB1000000,    /**< @h2xmle_name{StreamTX} */
        EQUALIZER_SWITCH    = 0xB2000000,    /**< @h2xmle_name{Equalizer} */
        VSID                = 0xB3000000,    /**< @h2xmle_name{VSID} */
        BT_PROFILE          = 0xB4000000,    /**< @h2xmle_name{BtProfile} */
        BT_FORMAT           = 0xB5000000,    /**< @h2xmle_name{BtFormat} */
        PBE_SWITCH          = 0xB6000000,    /**< @h2xmle_name{PBE_Switch} */
        BASS_BOOST_SWITCH   = 0xB7000000,    /**< @h2xmle_name{BASS_BOOST_Switch} */
        REVERB_SWITCH       = 0xB8000000,    /**< @h2xmle_name{Reverb_Switch} */
        VIRTUALIZER_SWITCH  = 0xB9000000,    /**< @h2xmle_name{Virtualizer_Switch} */
        SW_SIDETONE         = 0xBA000000,    /**< @h2xmle_name{SW_Sidetone} */
        TAG_KEY_SLOW_TALK   = 0xBB000000,    /**< @h2xmle_name{Stream_SlowTalk} */
        VOICE_UI_STREAM_CONFIG  = 0xBC000000,    /**< @h2xmle_name{VoiceUI_Stream_Config} */
        TAG_KEY_MUX_DEMUX_CONFIG   = 0xBD000000,    /**< @h2xmle_name{Stream_MuxDemux} */
        SPK_PRO_DEV_MAP      = 0xBE000000,    /**< @h2xmle_name{SP_Dev_Map} */
        SPK_PRO_VI_MAP      = 0xBF000000,    /**< @h2xmle_name{SP_VI_Map} */
        RAS_SWITCH          = 0xD0000000,    /**< @h2xmle_name{RAS_Switch} */
        PROXY_TX_TYPE          = 0xD1000000,    /**< @h2xmle_name{ProxyTxType} */
};

/**
        @h2xmlk_key {STREAMRX}
        @h2xmlk_description {Type of Rx Stream}
*/
enum Key_StreamRX {
        PCM_DEEP_BUFFER                 = 0xA1000001,    /**< @h2xmle_name {PCM_Deep_Buffer}*/
        PCM_RX_LOOPBACK                 = 0xA1000003,    /**< @h2xmle_name {PCM_Rx_Loopback}*/
        VOIP_RX_PLAYBACK                = 0xA1000005,    /**< @h2xmle_name {Voip_Rx}*/
        COMPRESSED_OFFLOAD_PLAYBACK     = 0xA100000A,    /**< @h2xmle_name {Compress_Offload_Playback}*/
        HFP_RX_PLAYBACK                 = 0xA100000C,    /**< @h2xmle_name {HFP_Rx_Playback}*/
        HFP_TX_PLAYBACK                 = 0xA100000D,    /**< @h2xmle_name {HFP_Tx_Playback}*/
        PCM_LL_PLAYBACK                 = 0xA100000E,    /**< @h2xmle_name {PCM_LL_Playback}*/
        PCM_OFFLOAD_PLAYBACK            = 0xA100000F,    /**< @h2xmle_name {PCM_Offload}*/
        VOICE_CALL_RX                   = 0xA1000010,    /**< @h2xmle_name {Voice_Call_Rx}*/
        PCM_ULL_PLAYBACK                = 0xA1000011,    /**< @h2xmle_name {PCM_ULL_Playback}*/
        PCM_PROXY_PLAYBACK              = 0xA1000012,    /**< @h2xmle_name {PCM_Proxy_Playback}*/
        INCALL_MUSIC                    = 0xA1000013,    /**< @h2xmle_name {Incall_Music}*/
};

/**
        @h2xmlk_key {STREAMTX}
        @h2xmlk_description {Type of Tx Stream}
*/
enum Key_StreamTX {
        PCM_RECORD      = 0xB1000001,    /**< @h2xmle_name {PCM_Record}*/
        PCM_TX_LOOPBACK = 0xB1000002,    /**< @h2xmle_name {PCM_Tx_Loopback}*/
        VOICE_UI        = 0xB1000003,    /**< @h2xmle_name {Voice_UI}*/
        VOIP_TX_RECORD  = 0xB1000004,    /**< @h2xmle_name {Voip_Tx}*/
        HFP_RX_CAPTURE  = 0xB1000005,    /**< @h2xmle_name {HFP_Rx_Capture}*/
        HFP_TX_CAPTURE  = 0xB1000006,    /**< @h2xmle_name {HFP_Tx_Capture}*/
        VOICE_CALL_TX   = 0xB1000007,    /**< @h2xmle_name {Voice_Call_Tx}*/
        DEEPBUFFER_RECORD  = 0xB1000008,    /**< @h2xmle_name {DeepBuffer_Record}*/
        RAW_RECORD         = 0xB1000009,    /**< @h2xmle_name {RAW_Record}*/
        PCM_ULL_RECORD  = 0xB100000A,    /**< @h2xmle_name {PCM_ULL_Record}*/
        PCM_PROXY_RECORD = 0xB100000B,   /**< @h2xmle_name {PCM_Proxy_Record}*/
        INCALL_RECORD   = 0xB100000C,    /**< @h2xmle_name {Incall_Record}*/
};

/**
    @h2xmlk_key {INSTANCE}
    @h2xmlk_description {Stream Instance Id}
*/
enum Key_Instance {
    INSTANCE_1 = 1, /**< @h2xmle_name {Instance_1}*/
    INSTANCE_2 = 2, /**< @h2xmle_name {Instance_2}*/
    INSTANCE_3 = 3, /**< @h2xmle_name {Instance_3}*/
    INSTANCE_4 = 4, /**< @h2xmle_name {Instance_4}*/
    INSTANCE_5 = 5, /**< @h2xmle_name {Instance_5}*/
    INSTANCE_6 = 6, /**< @h2xmle_name {Instance_6}*/
    INSTANCE_7 = 7, /**< @h2xmle_name {Instance_7}*/
    INSTANCE_8 = 8, /**< @h2xmle_name {Instance_8}*/
};

/**
    @h2xmlk_key {DEVICERX}
    @h2xmlk_description {Rx Device}
*/
enum Key_DeviceRX {
    SPEAKER = 0xA2000001, /**< @h2xmle_name {Speaker}*/
    HEADPHONES = 0xA2000002, /**< @h2xmle_name {Headphones}*/
    BT_RX      = 0xA2000003, /**< @h2xmle_name {BT_Rx}*/
    HANDSET    = 0xA2000004, /**< @h2xmle_name {Handset}*/
    USB_RX     = 0xA2000005, /**< @h2xmle_name {USB_Rx}*/
    HDMI_RX    = 0xA2000006, /**< @h2xmle_name {HDMI_Rx}*/
    PROXY_RX   = 0xA2000007, /**< @h2xmle_name {Proxy_Rx}*/
};
/**
    @h2xmlk_key {DEVICETX}
    @h2xmlk_description {Tx Device}
*/
enum Key_DeviceTX {
     SPEAKER_MIC     = 0xA3000001, /**< @h2xmle_name {Speaker_Mic}*/
     BT_TX           = 0xA3000002, /**< @h2xmle_name {BT_Tx}*/
     HEADPHONE_MIC   = 0xA3000003, /**< @h2xmle_name {Headphone_Mic}*/
     HANDSETMIC      = 0xA3000004, /**< @h2xmle_name {Handset_Mic}*/
     USB_TX          = 0xA3000005, /**< @h2xmle_name {USB_Tx}*/
     HANDSETMIC_VA   = 0xA3000006, /**< @h2xmle_name {VoiceActivation_Mic}*/
     HEADSETMIC_VA   = 0xA3000007, /**< @h2xmle_name {VoiceActivation_Headset_Mic}*/
     PROXY_TX        = 0xA3000008, /**< @h2xmle_name {Proxy_Tx} */
     VI_TX           = 0xA3000009, /**< @h2xmle_name {VI_TX} */
};

/**
        @h2xmlk_key {DEVICEPP_RX}
        @h2xmlk_description {Rx Device Post/Pre Processing Chain}
*/
enum Key_DevicePP_RX {
        DEVICEPP_RX_DEFAULT            = 0xAC000001, /**< @h2xmle_name {DevicePP_Rx_Default} @h2xmlk_description {Default PP}*/
        DEVICEPP_RX_AUDIO_MBDRC        = 0xAC000002, /**< @h2xmle_name {Audio_MBDRC} @h2xmlk_description {Audio Rx MBDRC PP }*/
        DEVICEPP_RX_VOIP_MBDRC         = 0xAC000003, /**< @h2xmle_name {Voip_MBDRC} @h2xmlk_description {Voip Rx MBDRC}*/
        DEVICEPP_RX_HFPSINK            = 0xAC000004, /**< @h2xmle_name {HFP_Sink} @h2xmlk_description {HFP Sink}*/
        DEVICEPP_RX_VOICE_DEFAULT      = 0xAC000005, /**< @h2xmle_name {DevicePP_RX_Voice_Default} @h2xmlk_description {Voice call default}*/
};
/**
        @h2xmlk_key {DEVICEPP_TX}
        @h2xmlk_description {Tx Device Post/Pre Processing Chain}
*/
enum Key_DevicePP_TX {
    DEVICEPP_TX_VOICE_UI_FLUENCE_FFECNS = 0xAD000001, /**< @h2xmle_name {Voice_UI_Fluence_FFECNS} @h2xmlk_description {Used in Voice UI use-cases}*/
    DEVICEPP_TX_AUDIO_FLUENCE_SMECNS    = 0xAD000002, /**< @h2xmle_name {Audio_Fluence_SMECNS} @h2xmlk_description {Single Mic ECNS }*/
    DEVICEPP_TX_AUDIO_FLUENCE_ENDFIRE   = 0xAD000003, /**< @h2xmle_name {Audio_Fluence_Endfire} @h2xmlk_description {EndFire_ECNS - Typically used for dual mic capture scenarios}*/
    DEVICEPP_TX_AUDIO_FLUENCE_PRO       = 0xAD000004, /**< @h2xmle_name {Audio_Fluence_Pro} @h2xmlk_description {Audio Multi MIC scenarios ; at least 3 or more Micss}*/
    DEVICEPP_TX_VOIP_FLUENCE_PRO        = 0xAD000005, /**< @h2xmle_name {Voip_Fluence_Pro} @h2xmlk_description {Voip Multi MIC scenarios ; at least 3 or more Micss}*/
    DEVICEPP_TX_HFP_SINK_FLUENCE_SMECNS = 0xAD000006, /**< @h2xmle_name {HFP_Sink_Fluence_SMECNS} @h2xmlk_description {HFP Sink Single Mic ECNS }*/
    DEVICEPP_TX_VOIP_FLUENCE_SMECNS     = 0xAD000007, /**< @h2xmle_name {Voip_Fluence_SMECNS} @h2xmlk_description {Voip SMECNS scenarios ;1 mic}*/
    DEVICEPP_TX_VOICE_FLUENCE_SMECNS    = 0xAD000008, /**< @h2xmle_name {Voice_Fluence_SMECNS} @h2xmlk_description {Single Mic ECNS in voice call}*/
    DEVICEPP_TX_VOICE_FLUENCE_ENDFIRE   = 0xAD000009, /**< @h2xmle_name {Voice_Fluence_Endfire} @h2xmlk_description {EndFire_ECNS - Typically used for dual mic capture scenarios in voice call}*/
    DEVICEPP_TX_VOICE_FLUENCE_PRO       = 0xAD00000A, /**< @h2xmle_name {Voice_Fluence_Pro} @h2xmlk_description {Multi MIC scenarios ; at least 3 or more Micss in a voice call}*/
    DEVICEPP_TX_VOICE_UI_FLUENCE_FFNS   = 0xAD00000B, /**< @h2xmle_name {Voice_UI_Fluence_FFNS} @h2xmlk_description {Used in Voice UI use-cases}*/
    DEVICEPP_TX_VOICE_UI_RAW            = 0xAD00000C, /**< @h2xmle_name {Voice_UI_RAW} @h2xmlk_description {Used in Voice UI use-cases}*/
    DEVICEPP_TX_VOIP_FLUENCE_ENDFIRE    = 0xAD00000D, /**< @h2xmle_name {Voip_Fluence_Endfire} @h2xmlk_description {Voip Endfire scenarios ;2 mic}*/
};

/**
        @h2xmlk_key {STREAMPP_RX}
        @h2xmlk_description {Rx Stream Post/Pre Processing Chain}
*/
enum Key_StreamPP_RX {
        STREAMPP_RX_DEFAULT = 0xAF000001, /**< @h2xmle_name {StreamPP_Rx_Default} @h2xmlk_description {Default PP Playback}*/
};

/**
        @h2xmlk_key {STREAMPP_TX}
        @h2xmlk_description {Tx Stream Post/Pre Processing Chain}
*/
enum Key_StreamPP_TX {
        STREAMPP_TX_DEFAULT = 0xB0000001, /**< @h2xmle_name {StreamPP_Tx_Default} @h2xmlk_description {Default PP Capture}*/
};


/**
        @h2xmlk_key {VOLUME}
        @h2xmlk_description {Volume}
*/
enum Key_Volume {
    LEVEL_0 = 0, /**< @h2xmle_name {Level_0}*/
    LEVEL_1 = 1, /**< @h2xmle_name {Level_1}*/
    LEVEL_2 = 2, /**< @h2xmle_name {Level_2}*/
    LEVEL_3 = 3, /**< @h2xmle_name {Level_3}*/
    LEVEL_4 = 4, /**< @h2xmle_name {Level_4}*/
    LEVEL_5 = 5, /**< @h2xmle_name {Level_5}*/
    LEVEL_6 = 6, /**< @h2xmle_name {Level_6}*/
    LEVEL_7 = 7, /**< @h2xmle_name {Level_7}*/
    LEVEL_8 = 8, /**< @h2xmle_name {Level_8}*/
    LEVEL_9 = 9, /**< @h2xmle_name {Level_9}*/
    LEVEL_10 = 10, /**< @h2xmle_name {Level_10}*/
    LEVEL_11 = 11, /**< @h2xmle_name {Level_11}*/
    LEVEL_12 = 12, /**< @h2xmle_name {Level_12}*/
    LEVEL_13 = 13, /**< @h2xmle_name {Level_13}*/
    LEVEL_14 = 14, /**< @h2xmle_name {Level_14}*/
    LEVEL_15 = 15, /**< @h2xmle_name {Level_15}*/
};
/**
    @h2xmlk_key {SAMPLINGRATE}
    @h2xmlk_sampleRate
    @h2xmlk_description {Sampling Rate}
*/
enum Key_SamplingRate {
    SAMPLINGRATE_8K = 8000,   /**< @h2xmle_sampleRate{8000} @h2xmle_name {SR_8K}*/
    SAMPLINGRATE_16K = 16000, /**< @h2xmle_sampleRate{16000} @h2xmle_name {SR_16K}*/
    SAMPLINGRATE_22K = 22050, /**< @h2xmle_sampleRate{22050} @h2xmle_name {SR_22K}*/
    SAMPLINGRATE_24K = 24000, /**< @h2xmle_sampleRate{24000} @h2xmle_name {SR_24K}*/
    SAMPLINGRATE_32K = 32000, /**< @h2xmle_sampleRate{32000} @h2xmle_name {SR_32K}*/
    SAMPLINGRATE_44K = 44100, /**< @h2xmle_sampleRate{44100} @h2xmle_name {SR_44.1K}*/
    SAMPLINGRATE_48K = 48000, /**< @h2xmle_sampleRate{48000} @h2xmle_name {SR_48K}*/
    SAMPLINGRATE_96K = 96000, /**< @h2xmle_sampleRate{96000} @h2xmle_name {SR_96K}*/
    SAMPLINGRATE_192K = 192000, /**< @h2xmle_sampleRate{192000} @h2xmle_name {SR_192K}*/
    SAMPLINGRATE_384K = 384000, /**< @h2xmle_sampleRate{384000} @h2xmle_name {SR_384K}*/
};
/**
    @h2xmlk_key {BITWIDTH}
    @h2xmlk_description {Bit Width}
*/
enum Key_BitWidth {
    BITWIDTH_16 = 16, /**< @h2xmle_name {BW_16}*/
    BITWIDTH_24 = 24, /**< @h2xmle_name {BW_24}*/
    BITWIDTH_32 = 32, /**< @h2xmle_name {BW_32}*/
};
/**
    @h2xmlk_key {PAUSE}
    @h2xmlk_description {Pause}
*/
enum Key_Pause {
    OFF = 0, /**< @h2xmle_name {Off}*/
    ON = 1, /**< @h2xmle_name {On}*/
};
/**
    @h2xmlk_key {MUTE}
    @h2xmlk_description {Mute}
*/
enum Key_Mute {
    MUTE_OFF = 0, /**< @h2xmle_name {Off}*/
    MUTE_ON = 1, /**< @h2xmle_name {On}*/
};
/**
    @h2xmlk_key {CHANNELS}
    @h2xmlk_description {Channels}
*/
enum Key_Channels {
    CHANNELS_1 = 1,   /**< @h2xmle_name {CHS_1}*/
    CHANNELS_2 = 2,   /**< @h2xmle_name {CHS_2}*/
    CHANNELS_3 = 3,   /**< @h2xmle_name {CHS_3}*/
    CHANNELS_4 = 4,   /**< @h2xmle_name {CHS_4}*/
    CHANNELS_5 = 5,   /**< @h2xmle_name {CHS_5}*/
    CHANNELS_5_1 = 6, /**< @h2xmle_name {CHS_6}*/
    CHANNELS_6 = 6,   /**< @h2xmle_name {CHS_6}*/
    CHANNELS_7 = 7,   /**< @h2xmle_name {CHS_7}*/
    CHANNELS_8 = 8,   /**< @h2xmle_name {CHS_8}*/
};
/**
    @h2xmlk_key {FLUENCE}
    @h2xmlk_description {Fluence}
*/
enum Key_Fluence {
    FLUENCE_OFF = 0, /**< @h2xmle_name {Off}*/
    FLUENCE_ON = 1, /**< @h2xmle_name {On}*/
    FLUENCE_EC = 2, /**< @h2xmle_name {EC}*/
    FLUENCE_NS = 3, /**< @h2xmle_name {NS}*/
};

/**
@h2xmlk_key {MEDIA_FMT_ID}
@h2xmlk_description {MediaFmtID}
*/
enum Key_MediaFmtID {
    MEDIA_FMT_PCM = 0xAE000001,     /**< @h2xmle_name {PCM} @h2xmlk_description {PCM Media format}*/
    MEDIA_FMT_AAC = 0xAE000002,     /**< @h2xmle_name {AAC} @h2xmlk_description {AAC Media format}*/
    MEDIA_FMT_ALAC = 0xAE000003,    /**< @h2xmle_name {ALAC} @h2xmlk_description {ALAC Media format}*/
    MEDIA_FMT_APE = 0xAE000004,     /**< @h2xmle_name {APE} @h2xmlk_description {APE Media format}*/
    MEDIA_FMT_WMAPRO = 0xAE000005,  /**< @h2xmle_name {WMAPRO} @h2xmlk_description {WMAPRO Media format}*/
    MEDIA_FMT_WMASTD = 0xAE000006,  /**< @h2xmle_name {WMASTD} @h2xmlk_description {WMASTD Media format}*/
    MEDIA_FMT_FLAC = 0xAE000007,    /**< @h2xmle_name {FLAC} @h2xmlk_description {FLAC Media format}*/
    MEDIA_FMT_VORBIS = 0xAE000008,  /**< @h2xmle_name {VORBIS} @h2xmlk_description {VORBIS Media format}*/
    MEDIA_FMT_MP3 = 0xAE000009,     /**< @h2xmle_name {MP3} @h2xmlk_description {MP3 Media format}*/
    MEDIA_FMT_AMRWBPLUS = 0xAE00000A,  /**< @h2xmle_name {AMRWBPLUS} @h2xmlk_description {AMRWBPLUS Media format}*/
};

/**
    @h2xmlk_key {VSID}
    @h2xmlk_description {VSID}
*/
enum Key_VSID {
    VSID_DEFAULT    = 0xB3000001, /*VSID default*/
    VSID_VOICE1     = 0xB3000002, /*VSID 0x11C05000*/
    VSID_VOICE2     = 0xB3000003, /*VSID 0x11DC5000*/
    VSID_VOICE1_LB  = 0xB3000004, /*VSID 0x12006000*/
    VSID_VOICE2_LB  = 0xB3000005, /*VSID 0x121C6000*/
};

/**
    @h2xmlk_key {EQUALIZER_SWITCH}
    @h2xmlk_description {Equalizer_Switch}
*/
enum Key_EQUALIZER {
    EQUALIZER_OFF = 0, /**< @h2xmle_name {Off}*/
    EQUALIZER_ON = 1, /**< @h2xmle_name {On}*/
};

/**
    @h2xmlk_key {BT_PROFILE}
    @h2xmlk_description {BTProfile}
*/
enum Key_BtProfile {
    SCO  = 0xB4000001, /**< @h2xmle_name {SCO}*/
    A2DP = 0xB4000002, /**< @h2xmle_name {A2DP}*/
};

/**
    @h2xmlk_key {PROXY_TX_TYPE}
    @h2xmlk_description {ProxyTxType}
*/
enum Key_ProxyTxType {
    PROXY_TX_DEFAULT  = 0xD1000001, /**< @h2xmle_name {PROXY_TX_DEFAULT}*/
    PROXY_TX_WFD = 0xD1000002, /**< @h2xmle_name {PROXY_TX_WFD}*/
};

/**
    @h2xmlk_key {BT_FORMAT}
    @h2xmlk_description {BTFormat}
*/
enum Key_BtFormat {
    GENERIC       = 0xB5000001, /**< @h2xmle_name {GENERIC}*/
    LDAC          = 0xB5000002, /**< @h2xmle_name {LDAC}*/
    APTX_ADAPTIVE = 0xB5000003, /**< @h2xmle_name {APTX_ADAPTIVE}*/
    SWB           = 0xB5000004, /**< @h2xmle_name {SWB}*/
};

/**
    @h2xmlk_key {VIRTUALIZER_SWITCH}
    @h2xmlk_description {Virtualizer_Switch}
*/
enum Key_VIRTUALIZER {
    VIRTUALIZER_OFF = 0, /**< @h2xmle_name {Off}*/
    VIRTUALIZER_ON = 1, /**< @h2xmle_name {On}*/
};

/**
    @h2xmlk_key {REVERB_SWITCH}
    @h2xmlk_description {Reverb_Switch}
*/
enum Key_REVERB {
    REVERB_OFF = 0, /**< @h2xmle_name {Off}*/
    REVERB_ON = 1, /**< @h2xmle_name {On}*/
};

/**
    @h2xmlk_key {PBE_SWITCH}
    @h2xmlk_description {PBE_Switch}
*/
enum Key_PBE {
    PBE_OFF = 0, /**< @h2xmle_name {Off}*/
    PBE_ON = 1, /**< @h2xmle_name {On}*/
};

/**
    @h2xmlk_key {BASS_BOOST_SWITCH}
    @h2xmlk_description {Bass_Boost_Switch}
*/
enum Key_BASS_BOOST {
    BASS_BOOST_OFF = 0, /**< @h2xmle_name {Off}*/
    BASS_BOOST_ON = 1, /**< @h2xmle_name {On}*/
};

/**
    @h2xmlk_key {SW_SIDETONE}
    @h2xmlk_description {SW_SIDETONE}
*/
enum Key_SW_SIDETONE {
    SW_SIDETONE_ON = 0xBA000001, /**< @h2xmle_name {SW_Sidetone}*/
};

/**
    @h2xmlk_key {TAG_KEY_SLOW_TALK}
    @h2xmlk_description {Stream_SlowTalk}
*/
enum Key_Slow_Talk {
    TAG_VALUE_SLOW_TALK_OFF = 0, /**< @h2xmle_name {Off}*/
    TAG_VALUE_SLOW_TALK_ON = 1, /**< @h2xmle_name {On}*/
};

/**
    @h2xmlk_key {VOICE_UI_STREAM_CONFIG}
    @h2xmlk_description {VuiStreamConfig}
*/
enum Key_VuiStreamConfig {
    VUI_STREAM_CFG_SVA           = 0xBC000001, /**< @h2xmle_name {SVA}*/
    VUI_STREAM_CFG_HW            = 0xBC000002, /**< @h2xmle_name {Hotword}*/
};

/**
    @h2xmlk_key {TAG_KEY_MUX_DEMUX_CONFIG}
    @h2xmlk_description {Stream_MuxDemux}
*/
enum Key_Mux_Demux_Config {
    TAG_VALUE_MUX_DEMUX_CONFIG_UPLINK = 1, /**< @h2xmle_name {UL_Mono}*/
    TAG_VALUE_MUX_DEMUX_CONFIG_DOWNLINK = 2, /**< @h2xmle_name {DL_Mono}*/
    TAG_VALUE_MUX_DEMUX_CONFIG_UPLINK_DOWNLINK_MONO = 3, /**< @h2xmle_name {UL_DL_Mono}*/
    TAG_VALUE_MUX_DEMUX_CONFIG_UPLINK_DOWNLINK_STEREO = 4, /**< @h2xmle_name {UL_DL_Stereo}*/
};

/**
    @h2xmlk_key {SPK_PRO_DEV_MAP}
    @h2xmlk_description {SpkrProtDevMap}
*/
enum Key_SpkrProtDevMap {
    SP_DISABLED = 0, /**< @h2xmle_name {Disable_SP}*/
    RIGHT_MONO  = 1, /**< @h2xmle_name {Right_Mono}*/
    LEFT_MONO   = 2,  /**< @h2xmle_name {Left_Mono}*/
    LEFT_RIGHT  = 3, /**< @h2xmle_name {Left_Right}*/
};

/**
    @h2xmlk_key {SPK_PRO_VI_MAP}
    @h2xmlk_description {SpkrProtVIMap}
*/
enum Key_SpkrProtVIState {
    RIGHT_SPKR   = 0, /**< @h2xmle_name {RightSpeaker}*/
    LEFT_SPKR    = 1, /**< @h2xmle_name {LeftSpeaker}*/
    STEREO_SPKR  = 2, /**< @h2xmle_name {StereoSpeaker}*/
};


/**
    @h2xmlk_key {RAS_SWITCH}
    @h2xmlk_description {RAS_Switch}
*/
enum Key_RAS {
    RAS_OFF = 0, /**< @h2xmle_name {Off}*/
    RAS_ON = 1, /**< @h2xmle_name {On}*/
};

/**
    @h2xmlk_gkeys
    @h2xmlk_description {Graph Keys}
*/
enum Graph_Keys {
        gk_StreamRX = STREAMRX,
        gk_DeviceRX = DEVICERX,
        gk_DeviceTX = DEVICETX,
        gk_DevicePP_RX = DEVICEPP_RX,
        gk_DevicePP_TX = DEVICEPP_TX,
        gk_Instance = INSTANCE,
        gk_StreamPP_RX = STREAMPP_RX,
        gk_StreamPP_TX = STREAMPP_TX,
        gk_StreamTX = STREAMTX,
        gk_VSID = VSID,
        gk_BtProfile = BT_PROFILE,
        gk_BtFormat = BT_FORMAT,
        gk_SWSidetone = SW_SIDETONE,
        gk_VUIStreamConfig = VOICE_UI_STREAM_CONFIG,
    gk_ProxyTxType = PROXY_TX_TYPE,
};
/**
    @h2xmlk_ckeys
    @h2xmlk_description {Calibration Keys}
*/
enum Cal_Keys {
    ck_volume = VOLUME,
    ck_channels = CHANNELS,
    ck_spkrprot = SPK_PRO_DEV_MAP,
    ck_ras        = RAS_SWITCH,
    ck_spvistate = SPK_PRO_VI_MAP,
};

#define DEVICE_HW_ENDPOINT_RX        0xC0000004
/**
    @h2xmlk_modTag {"device_hw_ep_rx",DEVICE_HW_ENDPOINT_RX}
    @h2xmlk_description {Hw EP Rx}
*/
enum HW_ENDPOINT_RX_Keys {
    tk1_hweprx = CHANNELS,
};
#define DEVICE_HW_ENDPOINT_TX        0xC0000005
/**
    @h2xmlk_modTag {"device_hw_ep_tx",DEVICE_HW_ENDPOINT_TX}
    @h2xmlk_description {Hw EP Tx}
*/
enum HW_ENDPOINT_TX_Keys {
    tk1_hweptx = CHANNELS,
};
#define TAG_PAUSE       0xC0000006
/**
    @h2xmlk_modTag {"stream_pause", TAG_PAUSE}
    @h2xmlk_description {Stream Pause}
*/
enum TAG_PAUSE_Keys {
    tk1_Pause = PAUSE,
};
#define TAG_MUTE        0xC0000007
/**
    @h2xmlk_modTag {"stream_mute", TAG_MUTE}
    @h2xmlk_description {Stream Mute}
*/
enum TAG_MUTE_Keys {
    tk1_Mute = MUTE,
};

#define TAG_FLUENCE  0xC000000A
/**
    @h2xmlk_modTag {"device_fluence", TAG_FLUENCE}
    @h2xmlk_description {Fluence On/Off}
*/
enum TAG_FLUENCE_Keys {
    tk1_Fluence = FLUENCE,
};

#define TAG_STREAM_VOLUME  0xC000000D
/**
    @h2xmlk_modTag {"stream_volume", TAG_STREAM_VOLUME}
    @h2xmlk_description {Stream Volume}
*/
enum TAG_STREAM_VOLUME_Keys {
    tk1_Volume = VOLUME,
};

#define TAG_DEVICE_PP_MFC  0xC0000011
/**
    @h2xmlk_modTag {"device_pp_mfc", TAG_DEVICE_PP_MFC}
    @h2xmlk_description {Device PP MFC}
*/
enum TAG_DEVICE_PP_MFC_Keys {
        tk1_device_pp_mfc = SAMPLINGRATE,
        tk2_device_pp_mfc = BITWIDTH,
        tk3_device_pp_mfc = CHANNELS,
};

#define TAG_STREAM_MFC  0xC000000B
/**
    @h2xmlk_modTag {"stream_mfc", TAG_STREAM_MFC}
    @h2xmlk_description {Stream MFC}
*/
enum TAG_STREAM_MFC_Keys {
    tk1_StreamSamplingRate = SAMPLINGRATE,
    tk2_StreamBitWidth = BITWIDTH,
    tk3_StreamChannels = CHANNELS,
};

#define TAG_STREAM_PLACEHOLDER_DECODER  0xC0000012
/**
@h2xmlk_modTag {"stream_placeholder_decoder", TAG_STREAM_PLACEHOLDER_DECODER}
@h2xmlk_description {Placeholder Decoder}
*/
enum TAG_STREAM_PLACEHOLDER_DECODER_Keys {
    tk1_MediaFmtID = MEDIA_FMT_ID,
};

#define TAG_STREAM_EQUALIZER  0xC0000014
/**
@h2xmlk_modTag {"stream_equalizer", TAG_STREAM_EQUALIZER}
@h2xmlk_description {Stream Equalizer}
*/
enum TAG_STREAM_EQUALIZER_Keys {
    tk1_EQUALIZER = EQUALIZER_SWITCH,
};


#define TAG_STREAM_VIRTUALIZER  0xC0000015
/**
@h2xmlk_modTag {"stream_virtualizer", TAG_STREAM_VIRTUALIZER}
@h2xmlk_description {Stream Virtualizer}
*/
enum TAG_STREAM_VIRTUALIZER_Keys {
    tk1_VIRTUALIZER = VIRTUALIZER_SWITCH,
};

#define TAG_STREAM_REVERB  0xC0000016
/**
@h2xmlk_modTag {"stream_reverb", TAG_STREAM_REVERB}
@h2xmlk_description {Stream Reverb}
*/
enum TAG_STREAM_REVERB_Keys {
    tk1_REVERB = REVERB_SWITCH,
};

#define TAG_STREAM_PBE  0xC0000017
/**
@h2xmlk_modTag {"stream_pbe", TAG_STREAM_PBE}
@h2xmlk_description {Stream PBE}
*/
enum TAG_STREAM_PBE_Keys {
    tk1_PBE = PBE_SWITCH,
};

#define TAG_STREAM_BASS_BOOST  0xC0000018
/**
@h2xmlk_modTag {"stream_bass_bost", TAG_STREAM_BASS_BOOST}
@h2xmlk_description {Stream Bass Boost}
*/
enum TAG_STREAM_BASS_BOOST_Keys {
    tk1_BASS_BOOST = BASS_BOOST_SWITCH,
};

#define PER_STREAM_PER_DEVICE_MFC  0xC0000019

/**
        @h2xmlk_modTag {"pspd_mfc", PER_STREAM_PER_DEVICE_MFC}
        @h2xmlk_description {Per Stream Per Device MFC}
*/

enum TAG_PER_STREAM_PER_DEVICE_MFC_Keys {
        tk1_pspd_mfc = SAMPLINGRATE,
        tk2_pspd_mfc = BITWIDTH,
        tk3_pspd_mfc = CHANNELS,
};

#define TAG_STREAM_SLOW_TALK  0xC0000025
/**
    @h2xmlk_modTag {"stream_slowtalk", TAG_STREAM_SLOW_TALK}
    @h2xmlk_description {Slow Talk}
*/
enum TAG_STREAM_SLOW_TALK_keys {
    tk1_slowtalk = TAG_KEY_SLOW_TALK,
};

#define TAG_MODULE_CHANNELS  0xC0000026
/**
    @h2xmlk_modTag {"module_channels", TAG_MODULE_CHANNELS}
    @h2xmlk_description {Module Channels}
*/
enum TAG_MODULE_CHANNELS_keys {
    tk1_module_channels = CHANNELS,
};

#define TAG_STREAM_MUX_DEMUX  0xC0000027
/**
    @h2xmlk_modTag {"stream_muxdemux", TAG_STREAM_MUX_DEMUX}
    @h2xmlk_description {Mux Demux}
*/
enum TAG_STREAM_MUX_DEMUX_keys {
    tk1_muxdemux = TAG_KEY_MUX_DEMUX_CONFIG,
};

#define TAG_STREAM_PLACEHOLDER_ENCODER  0xC000002A

/**
    @h2xmlk_modTagList
*/
enum TAGS_DEFINITIONS {
    SHMEM_ENDPOINT              = 0xC0000001, /**< @h2xmle_name {"sh_ep"} */
    STREAM_INPUT_MEDIA_FORMAT   = 0xC0000002, /**< @h2xmle_name {"stream_input_media_format" } */
    STREAM_OUTPUT_MEDIA_FORMAT  = 0xC0000003, /**< @h2xmle_name {"stream_output_media_format" } */
    DEVICE_SVA                  = 0xC0000008, /**< @h2xmle_name {"device_sva"} */
    DEVICE_ADAM                 = 0xC0000009, /**< @h2xmle_name {"device_adam"} */
    DEVICE_MFC                  = 0xC000000C, /**< @h2xmle_name {"device_mfc"} */
    STREAM_PCM_DECODER          = 0xC000000E, /**< @h2xmle_name {"stream_pcm_decoder"} */
    STREAM_PCM_ENCODER          = 0xC000000F, /**< @h2xmle_name {"stream_pcm_encoder"} */
    STREAM_PCM_CONVERTER        = 0xC0000010, /**< @h2xmle_name {"stream_pcm_converter"} */
    STREAM_SPR                  = 0xC0000013, /**< @h2xmle_name {"stream_spr"} */
    BT_PLACEHOLDER_ENCODER      = 0xC0000020, /**< @h2xmle_name {"bt_encoder"} */
    COP_PACKETIZER_V0           = 0xC0000021, /**< @h2xmle_name {"cop_packetizer_v0"} */
    RAT_RENDER                  = 0xC0000022, /**< @h2xmle_name {"rate_adapter_module"} */
    BT_PCM_CONVERTER            = 0xC0000023, /**< @h2xmle_name {"bt_pcm_converter"} */
    BT_PLACEHOLDER_DECODER      = 0xC0000024, /**< @h2xmle_name {"bt_decoder"} */
    MODULE_VI                   = 0xC0000028, /**< @h2xmle_name {"module_vi"} */
    MODULE_SP                   = 0xC0000029, /**< @h2xmle_name {"module_sp"} */
};
typedef enum TAGS_DEFINITIONS TAGS_DEFINITIONS;

#endif
