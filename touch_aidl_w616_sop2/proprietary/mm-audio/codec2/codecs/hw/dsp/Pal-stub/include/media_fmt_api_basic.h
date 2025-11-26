#ifndef MEDIA_FMT_API_BASIC_H
#define MEDIA_FMT_API_BASIC_H
/**
 * \file media_fmt_api.h
 * \brief
 *           This file contains media format IDs and definitions
 *
 * \copyright
 *  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/modules/media_fmt_api_basic.h#7 $
*/
// clang-format on

#include "ar_defs.h"
#include "media_fmt_api_ext.h"

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

/** @h2xml_title1           {Media Format APIs}
    @h2xml_title_agile_rev  {Media Format APIs}
    @h2xml_title_date       {August 13, 2018} */
/**
   @h2xmlx_xmlNumberFormat {int}
*/


/** Infinity */
#define INFINITE                         -1

/** Zero is invalid value */
#define INVALID_VALUE                     0

/*********************************************** Channel Map Values ***************************************************/
/** Front left channel. */
enum pcm_channel_map
{
   /** Front left channel. */
   PCM_CHANNEL_L = 1,

   /** Front right channel. */
   PCM_CHANNEL_R = 2,

   /** Front center channel. */
   PCM_CHANNEL_C = 3,

   /** Left surround channel.*/
   PCM_CHANNEL_LS = 4,

   /** Right surround channel. */
   PCM_CHANNEL_RS = 5,

   /** Low frequency effect channel. */
   PCM_CHANNEL_LFE = 6,

   /** Center surround channel;
    * rear center channel. */
   PCM_CHANNEL_CS = 7,

   /** Center back channel. */
   PCM_CHANNEL_CB = PCM_CHANNEL_CS,

   /** Left back channel;
    * rear left channel. */
   PCM_CHANNEL_LB = 8,

   /** Right back channel;
    * rear right channel. */
   PCM_CHANNEL_RB = 9,

   /** Top surround channel. */
   PCM_CHANNEL_TS = 10,

   /** Center vertical height channel. */
   PCM_CHANNEL_CVH = 11,

   /** Top front center channel. */
   PCM_CHANNEL_TFC = PCM_CHANNEL_CVH,

   /** Mono surround channel. */
   PCM_CHANNEL_MS = 12,

   /** Front left of center channel. */
   PCM_CHANNEL_FLC = 13,

   /** Front right of center channel. */
   PCM_CHANNEL_FRC = 14,

   /** Rear left of center channel. */
   PCM_CHANNEL_RLC = 15,

   /** Rear right of center channel. */
   PCM_CHANNEL_RRC = 16,

   /** Secondary low frequency effect channel. */
   PCM_CHANNEL_LFE2 = 17,

   /** Side left channel. */
   PCM_CHANNEL_SL = 18,

   /** Side right channel. */
   PCM_CHANNEL_SR = 19,

   /** Top front left channel. */
   PCM_CHANNEL_TFL = 20,

   /** Left vertical height channel. */
   PCM_CHANNEL_LVH = PCM_CHANNEL_TFL,

   /** Top front right channel. */
   PCM_CHANNEL_TFR = 21,

   /** Right vertical height channel. */
   PCM_CHANNEL_RVH = PCM_CHANNEL_TFR,

   /** Top center channel. */
   PCM_CHANNEL_TC = 22,

   /** Top back left channel. */
   PCM_CHANNEL_TBL = 23,

   /** Top back right channel. */
   PCM_CHANNEL_TBR = 24,

   /** Top side left channel. */
   PCM_CHANNEL_TSL = 25,

   /** Top side right channel. */
   PCM_CHANNEL_TSR = 26,

   /** Top back center channel. */
   PCM_CHANNEL_TBC = 27,

   /** Bottom front center channel. */
   PCM_CHANNEL_BFC = 28,

   /** Bottom front left channel. */
   PCM_CHANNEL_BFL = 29,

   /** Bottom front right channel. */
   PCM_CHANNEL_BFR = 30,

   /** Left wide channel. */
   PCM_CHANNEL_LW = 31,

   /** Right wide channel. */
   PCM_CHANNEL_RW = 32,

   /** Left side direct channel. */
   PCM_CHANNEL_LSD = 33,

   /** Right side direct channel. */
   PCM_CHANNEL_RSD = 34,

   /** Channel map 48 to 63 are reserved for custom channel maps */
   PCM_CUSTOM_CHANNEL_MAP_1  = 48,
   PCM_CUSTOM_CHANNEL_MAP_2  = 49,
   PCM_CUSTOM_CHANNEL_MAP_3  = 50,
   PCM_CUSTOM_CHANNEL_MAP_4  = 51,
   PCM_CUSTOM_CHANNEL_MAP_5  = 52,
   PCM_CUSTOM_CHANNEL_MAP_6  = 53,
   PCM_CUSTOM_CHANNEL_MAP_7  = 54,
   PCM_CUSTOM_CHANNEL_MAP_8  = 55,
   PCM_CUSTOM_CHANNEL_MAP_9  = 56,
   PCM_CUSTOM_CHANNEL_MAP_10 = 57,
   PCM_CUSTOM_CHANNEL_MAP_11 = 58,
   PCM_CUSTOM_CHANNEL_MAP_12 = 59,
   PCM_CUSTOM_CHANNEL_MAP_13 = 60,
   PCM_CUSTOM_CHANNEL_MAP_14 = 61,
   PCM_CUSTOM_CHANNEL_MAP_15 = 62,
   PCM_CUSTOM_CHANNEL_MAP_16 = 63,

   PCM_MAX_CHANNEL_MAP = 63
};
/*********************************************** Bits Per Sample Values ***********************************************/
/* Bits per sample (= sample word size) */
#define BITS_PER_SAMPLE_16        16
#define BITS_PER_SAMPLE_24        24
#define BITS_PER_SAMPLE_32        32

/********************************************** Bytes Per Sample Values ***********************************************/
/* Bytes per sample */
#define BYTES_PER_SAMPLE_TWO              2
#define BYTES_PER_SAMPLE_THREE            3
#define BYTES_PER_SAMPLE_FOUR             4

/********************************************** Bit Width Values ******************************************************/
/* Bit width (actual width of the sample in a word) */
#define BIT_WIDTH_16                16
#define BIT_WIDTH_24                24
#define BIT_WIDTH_32                32

/********************************************** Alignment Values ******************************************************/
/**
 * Alignment
 */
#define PCM_LSB_ALIGNED                   1
#define PCM_MSB_ALIGNED                   2

/********************************************** Q Factor Values *******************************************************/
/**
 * Q factors
 */
#define PCM_Q_FACTOR_15                   15
#define PCM_Q_FACTOR_23                   23
#define PCM_Q_FACTOR_27                   27
#define PCM_Q_FACTOR_31                   31

/** Shift factor for Q31 <=> Q28 conversion for 32-bit P
*/
#define PCM_QFORMAT_SHIFT_FACTOR         (PCM_Q_FACTOR_31 - PCM_Q_FACTOR_27)

/********************************************** Sampling Rates ********************************************************/
/** Sample rate is 8 kHz. */
#define SAMPLE_RATE_8K                    8000

/** Sample rate is 11.025 kHz. */
#define SAMPLE_RATE_11_025K               11025

/** Sample rate is 12 kHz. */
#define SAMPLE_RATE_12K                   12000

/** Sample rate is 16 kHz. */
#define SAMPLE_RATE_16K                   16000

/** Sample rate is 22.05 kHz. */
#define SAMPLE_RATE_22_05K                22050

/** Sample rate is 24 kHz. */
#define SAMPLE_RATE_24K                   24000

/** Sample rate is 32 kHz. */
#define SAMPLE_RATE_32K                   32000

/** Sample rate is 44.1 kHz. */
#define SAMPLE_RATE_44_1K                 44100

/** Sample rate is 48 kHz. */
#define SAMPLE_RATE_48K                   48000

/** Sample rate is 88.2 kHz. */
#define SAMPLE_RATE_88_2K                 88200

/** Sample rate is 96 kHz. */
#define SAMPLE_RATE_96K                   96000

/** Sample rate is 176.4 kHz.*/
#define SAMPLE_RATE_176_4K                176400

/** Sample rate is 192 kHz. */
#define SAMPLE_RATE_192K                  192000

/** Sample rate is 352.8 kHz. */
#define SAMPLE_RATE_352_8K                352800

/** Sample rate is 384 kHz. */
#define SAMPLE_RATE_384K                  384000

/********************************************** Endianess Values*******************************************************/
/**
 * Endianness
 */
#define PCM_LITTLE_ENDIAN                 1
#define PCM_BIG_ENDIAN                    2

/********************************************** Interleaving Values*****************************************************/
/**
 * Interleaved PCM
 */
#define PCM_INTERLEAVED                   1
/**
 * Packed Deinterleaved PCM:
 * A buffer of max size M with C channels and N/C actual bytes per channel
 * is deinterleaved-packed if (M - N) is zero.
 */
#define PCM_DEINTERLEAVED_PACKED          2
/**
 * Unpacked Deinterleaved PCM:
 * A buffer of max size M with C channels and N/C actual bytes per channel
 * is deinterleaved-unpacked if (M - N) is nonzero.
 * OR each channel has its own buffers with actual length being less than max length.
 */
#define PCM_DEINTERLEAVED_UNPACKED        3

/********************************************** Data Formats **********************************************************/

/** Data format is fixed point */
#define DATA_FORMAT_FIXED_POINT           1

/** Data format is IEC61937 packetized stream
 * Data has properties such as sample rate, number of channels, bit-width like PCM etc */
#define DATA_FORMAT_IEC61937_PACKETIZED   2

/** Data format is IEC60958 packetized stream for PCM only
 * Data has properties such as sample rate, number of channels, bit-width like PCM etc*/
#define DATA_FORMAT_IEC60958_PACKETIZED   3

/** Data format is DSD over PCM stream
 * Data has properties such as sample rate, number of channels, bit-width like PCM etc*/
#define DATA_FORMAT_DSD_OVER_PCM          4

/** Data format is generic compressed stream
 * Data has properties such as sample rate, number of channels, bit-width like PCM etc*/
#define DATA_FORMAT_GENERIC_COMPRESSED    5

/** Data format is raw compressed stream.
 * Data is raw compressed bit stream. Data does NOT have properties
 * such as sample rate, number of channels and bit-width like PCM */
#define DATA_FORMAT_RAW_COMPRESSED        6

/** Compressed bitstreams packetized like PCM using a QTI-designed packetizer*/
#define DATA_FORMAT_COMPR_OVER_PCM_PACKETIZED     7

/** Data format is IEC60958 packetized stream for compressed streams
 * Data has properties such as sample rate, number of channels, bit-width like PCM etc*/
#define DATA_FORMAT_IEC60958_PACKETIZED_NON_LINEAR 8

/********************************************** Configuration Modes****************************************************/
/** Configured parameter (like bps, num_channels, etc) will be a
   don't care - The module should continue to use the previously
   set configuration (or module defaults).
   E.g. if client wants to control num_channels from host but bits per sample from ACDB, then
   ACDB will have the value of -2. After graph-open, host can issue command with bits per sample as -2
   and num_channels with proper value*/
#define PARAM_VAL_UNSET     (-2)

/** Configured parameter (like bps, num_channels, etc) will need to follow
    the input media format that the module will receive or has received. */
#define PARAM_VAL_NATIVE    (-1)

/** Configured parameter (like bps, num_channels, etc) is considered invalid
    and the set_param should error out.
    This is used to detect explicit initialization vs. initialization by zero memset.*/
#define PARAM_VAL_INVALID     0

/************************* IEC61937_PACKETIZED and IEC60958_PACKETIZED data format ************************************/
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"

/**
 * Payload of data_format = DATA_FORMAT_IEC61937_PACKETIZED or DATA_FORMAT_IEC60958_PACKETIZED
 * Contains media format information independent of payload for packetized media_fmt_id
 */
struct payload_data_fmt_iec_packetized_t
{
   uint32_t sample_rate;
   /**< Number of samples per second.

        @values 0 to 192000 Hz */
   /**< @h2xmle_description {Sampling rate of audio stream}
        @h2xmle_default     {0}
        @h2xmle_rangeList   {"44.1 kHz"=44100;
                             "48 kHz"=48000;
                             "88.2 kHz"=88200;
                             "96 kHz"=96000;
                             "176.4 kHz"=176400;
                             "192 kHz"=192000}
        @h2xmle_policy      {Basic} */

   uint16_t num_channels;
   /**< @h2xmle_description {Number of channels}
        @h2xmle_default     {0}
        @h2xmle_rangeList   { "TWO"=2;
                              "EIGHT"=8 }
        @h2xmle_policy      {Basic} */

   uint16_t reserved;
   /**< @h2xmle_description {Used for alignment; must be set to 0.}
        @h2xmle_policy      {Basic} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct payload_data_fmt_iec_packetized_t payload_data_fmt_iec_packetized_t;

/************************* GENERIC_COMPRESSED data format ************************************/
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"

/**
 * Payload of data_format = DATA_FORMAT_GENERIC_COMPRESSED
 * Contains media format information independent of payload for generic compressed media_fmt_id
 */
struct payload_data_fmt_generic_compressed_t
{
   uint32_t sample_rate;
   /**< Number of samples per second.

        @values 0 to 384000 Hz */
   /**< @h2xmle_description {Sampling rate of audio stream}
        @h2xmle_default     {0}
        @h2xmle_rangeList   {"8 kHz"=8000;
                             "11.025 kHz"=11025;
                             "12 kHz"=12000;
                             "16 kHz"=16000;
                             "22.05 kHz"=22050;
                             "24 kHz"=24000;
                             "32 kHz"=32000;
                             "44.1 kHz"=44100;
                             "48 kHz"=48000;
                             "88.2 kHz"=88200;
                             "96 kHz"=96000;
                             "176.4 kHz"=176400;
                             "192 kHz"=192000;
                             "352.8 kHz"=352800;
                             "384 kHz"=384000}
        @h2xmle_policy      {Basic} */

   uint16_t bits_per_sample;
   /**< @h2xmle_description {Bits needed to store one sample.
                             - 16-bits per sample always contain 16-bit samples.
                             - 32-bits per sample always contain 32-bit samples }
        @h2xmle_default     {0}
        @h2xmle_rangeList   { "INVALID_VALUE"=0;
                              ""=16;
                              ""=32 }
        @h2xmle_policy      {Basic} */

   uint16_t num_channels;
   /**< @h2xmle_description {Number of channels}
        @h2xmle_default     {0}
        @h2xmle_range       { 0..32 }
        @h2xmle_policy      {Basic} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct payload_data_fmt_generic_compressed_t payload_data_fmt_generic_compressed_t;


/*************************************************** PCM Media Format *************************************************/
/**
 * Media format ID for identifying PCM streams.
 */
#define MEDIA_FMT_ID_PCM                  0x09001000

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"

/**
 * Payload of fmt-id = MEDIA_FMT_ID_PCM and data_format = DATA_FORMAT_FIXED_POINT
 *
 * struct complete_media_fmt_pcm_t
 * {
 *    payload_media_fmt_pcm_t pcm;
 *    uint8_t ch_map[num_channels];
 * }
 */
struct payload_media_fmt_pcm_t
{
   uint32_t sample_rate;
   /**< Number of samples per second.

        @values 0 to 384000 Hz */
   /**< @h2xmle_description {Sampling rate of audio stream}
        @h2xmle_default     {0}
        @h2xmle_rangeList   {"8 kHz"=8000;
                             "11.025 kHz"=11025;
                             "12 kHz"=12000;
                             "16 kHz"=16000;
                             "22.05 kHz"=22050;
                             "24 kHz"=24000;
                             "32 kHz"=32000;
                             "44.1 kHz"=44100;
                             "48 kHz"=48000;
                             "88.2 kHz"=88200;
                             "96 kHz"=96000;
                             "176.4 kHz"=176400;
                             "192 kHz"=192000;
                             "352.8 kHz"=352800;
                             "384 kHz"=384000}
        @h2xmle_policy      {Basic} */

   uint16_t bit_width;
   /**< @h2xmle_description {bit width of each sample. E.g. 16 bit, 24 bit or 32 bit}
        @h2xmle_default     {0}
        @h2xmle_rangeList   { "INVALID_VALUE"=0;
                              ""=16;
                              ""=24;
                              ""=32}
        @h2xmle_policy      {Basic} */

   uint8_t alignment;
   /**< @h2xmle_description {Indicates the alignment of bits_per_sample in sample_word_size. \n
                             Relevant only when bits_per_sample is 24 and word_size is 32}
        @h2xmle_default     {0}
        @h2xmle_rangeList   { "INVALID_VALUE"=0;
                              "PCM_LSB_ALIGNED"=1;
                              "PCM_MSB_ALIGNED"=2}
        @h2xmle_policy      {Basic} */

   uint8_t interleaved;
    /**< @h2xmle_description {Data is interleaved or not. Data is assumed to interleaved for invalid}
        @h2xmle_default     {0}
        @h2xmle_rangeList   { "INVALID_VALUE"=0;
                              "PCM_INTERLEAVED"=1}
        @h2xmle_policy      {Basic} */

   uint16_t bits_per_sample;
   /**<
        @h2xmle_description {Bits needed to store one sample.
                             - 16-bits per sample always contain 16-bit samples.
                             - 24-bits per sample always contain 24-bit samples.
                             - 32-bits per sample have below cases:
                               - If bit width = 24 and alignment = LSB aligned, then
                                  24-bit samples are placed in the lower 24 bits of a 32-bit word.
                                  Upper bits may or may not be sign-extended.
                               - If bit width = 24 and alignment = MSB aligned, then
                                  24-bit samples are placed in the upper 24 bits of a 32-bit word.
                                  Lower bits may or may not be zeroed.
                               - If bit width = 32, 32-bit samples are placed in the
                                 32-bit words
                            }
        @h2xmle_default     {0}
        @h2xmle_rangeList   { "INVALID_VALUE"=0;
                              ""=16;
                              ""=24;
                              ""=32
                            }
        @h2xmle_policy      {Basic} */

   uint16_t q_factor;
   /**< @h2xmle_description {Q factor of the PCM data.
                             15 for 16 bit signed data
                             23 for 24 bit signed packed (24 word size) data
                             27 for LSB aligned 24 bit unpacked (32 word size) signed data used internal to spf.
                             31 for MSB aligned 24 bit unpacked (32 word size) signed data
                             24 for LSB aligned 24 bit unpacked (32 word size) signed data
                             31 for 32 bit signed data}
        @h2xmle_default     {0}
        @h2xmle_rangeList   {
                              "INVALID_VALUE"=0;
                              ""=15;
                              ""=23;
                              ""=24;
                              ""=27;
                              ""=31
                            }
        @h2xmle_policy      {Basic} */


   uint16_t endianness;
   /**< @h2xmle_description {Indicates whether PCM samples are stored in little endian or big endian format.}
        @h2xmle_default     {0}
        @h2xmle_rangeList   { "INVALID_VALUE"=0;
                              "PCM_LITTLE_ENDIAN"=1;
                              "PCM_BIG_ENDIAN"=2
                            }
        @h2xmle_policy      {Basic} */

   uint16_t num_channels;
   /**< @h2xmle_description {Number of channels}
        @h2xmle_default     {0}
        @h2xmle_range       { 0..32 }
        @h2xmle_policy      {Basic} */

#if defined(__H2XML__)
   uint8_t channel_mapping[0];
   /**< @h2xmle_description {Channel mapping array of variable size.
                             Size of this array depends upon number of channels.
                             Channel[i] mapping describes channel i. Each element i of the array
                             describes channel i inside the buffer where i is less than num_channels.
                             An unused channel is set to 0.}
        h2xmle_variableArraySize {(num_channels*4 + 3) / 4}
        @h2xmle_default     {0}
        @h2xmle_policy      {Basic} */
#endif

}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct payload_media_fmt_pcm_t payload_media_fmt_pcm_t;


/********************************************** Media Format Parameter ************************************************/
/**
 *  Param that indicates media format of the future buffers in the stream.

    This param-id is accepted only when subgraph is in STOP/PREPATE state.

    Some decoders may not work without receiving either
    PARAM_ID_MEDIA_FORMAT or DATA_CMD_WR_SH_MEM_EP_MEDIA_FORMAT

    Payload is of type media_format_t

    This param-id is set using APM_CMD_SET_CFG
 */
#define PARAM_ID_MEDIA_FORMAT       0x0800100C

/** @h2xmlp_parameter   {"PARAM_ID_MEDIA_FORMAT", PARAM_ID_MEDIA_FORMAT}
    @h2xmlp_toolPolicy  {Calibration}
    @h2xmlp_description {Parameter for setting the media format on any Shared Memory End Point module.} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/**
 * Immediately following payload_size amount of bytes
 * which represent the actual media-fmt block
 *
 * Some opcodes that use this payload are:
 * -PARAM_ID_MEDIA_FORMAT
 * -DATA_CMD_WR_SH_MEM_EP_MEDIA_FORMAT
 * -DATA_EVENT_ID_RD_SH_MEM_EP_MEDIA_FORMAT
 *
 * Payload struct
 * struct
 * {
 *    media_format_t mf;
 *    uint8_t payload[payload_size];
 * }
 */
struct media_format_t
{
   uint32_t data_format;
    /**< @h2xmle_description {Format of the data}
         @h2xmle_default     {0}
         @h2xmle_rangeList   {"INVALID_VALUE"=0,
                              "DATA_FORMAT_FIXED_POINT"=1,
                              "DATA_FORMAT_IEC61937_PACKETIZED"=2,
                              "DATA_FORMAT_IEC60958_PACKETIZED"=3,
                              "DATA_FORMAT_DSD_OVER_PCM"=4,
                              "DATA_FORMAT_GENERIC_COMPRESSED"=5,
                              "DATA_FORMAT_RAW_COMPRESSED"=6}
         @h2xmle_policy      {Basic} */

   uint32_t fmt_id;
    /**< @h2xmle_description {Format ID of the data stream}
         @h2xmle_default     {0}
         @h2xmle_rangeList   {"INVALID_VALUE"=0,
                              "Media format ID of PCM"=MEDIA_FMT_ID_PCM}
         @h2xmle_policy      {Basic} */

   uint32_t payload_size;
    /**< @h2xmle_description {Size of the payload immediately following this structure.\n
                              The struct of payload is defined by combination of data_format and fmt_id.
                              E.g. PCM fixed point (payload_media_fmt_pcm_t) and floating point may have different payloads.
                              This size does not include bytes added for 32-bit alignment}
         @h2xmle_default     {0}
         @h2xmle_range       {0..0xFFFFFFFF}
         @h2xmle_policy      {Basic} */
#if defined(__H2XML__)
   uint8_t  payload[0];
    /**<
         @h2xmle_description {Payload}
         h2xmle_rangeList    {"For fmt_id = PARAM_ID_PCM_OUTPUT_FORMAT_CFG and data_format=DATA_FORMAT_FIXED_POINT"=payload_media_fmt_pcm_t}
         @h2xmle_policy      {Basic}
         h2xmle_variableArraySize { (payload_size*4+3) / 4}*/
#endif
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct media_format_t media_format_t;

/*********************************** Encoder Configuration Block (Encoder Output Config) ******************************/
/**
 * Param ID used to configure encoder
 *
 * All encoders may not support encoder output config
 *
 */
#define PARAM_ID_ENCODER_OUTPUT_CONFIG                   0x08001009


/** @h2xmlp_parameter   {"PARAM_ID_ENCODER_OUTPUT_CONFIG", PARAM_ID_ENCODER_OUTPUT_CONFIG}
    @h2xmlp_description {Param ID used to configure encoder.
                         Encoding happens at the incoming sample rate, channels and bit width.
                         These are controlled by either resamplers, MFC, channel mixer, or PCM converter.
                         Overall struct contains\n
                         - \n
                         --     param_id_encoder_output_config_t cfg;     \n
                         --     uint8_t custom_enc_cfg[payload_size];\n
                         --     uint8_t padding[if_any];\n
                         -\n
                         }
    @h2xmlp_toolPolicy  {Calibration} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/**
 * Structure for PARAM_ID_ENCODER_OUTPUT_CONFIG
 *
 * overall struct is
 * {
 *    param_id_encoder_output_config_t cfg;
 *    uint8_t custom_enc_cfg[payload_size];
 *    uint8_t padding[if_any];
 * }
 *
 * Encoding happens at the incoming sample rate, channels and bit width.
 * These are controlled by either resamplers, MFC, channel mixer, or PCM converter.
 *
 */
struct param_id_encoder_output_config_t
{
   uint32_t      data_format;
    /**< @h2xmle_description {Format of the data}
         @h2xmle_default     {0}
         @h2xmle_rangeList   {"PARAM_VAL_INVALID"=0,
                              "DATA_FORMAT_FIXED_POINT"=1}
         @h2xmle_policy      {Basic} */


   uint32_t       fmt_id;
    /**< @h2xmle_description {Format ID of the data stream.
                              For PCM encoding use case this must match the fmt_id in PARAM_ID_PCM_OUTPUT_FORMAT_CFG.}
         @h2xmle_default     {0}
         @h2xmle_rangeList   {"INVALID_VALUE"=0,
                                      "Media format ID of PCM"=MEDIA_FMT_ID_PCM}
                                                          @h2xmle_policy      {Basic} */

   uint32_t       payload_size;
    /**< @h2xmle_description {Size of the custom payload that follows this structure\n
                              the struct of payload is defined by combination of data_format and fmt_id.
                              E.g. PCM fixed point (payload_media_fmt_pcm_t) and floating point may have different payloads.
                              This size does not include bytes added for 32-bit alignment}
         @h2xmle_default     {0}
         @h2xmle_range       {0..0xFFFFFFFF}
         @h2xmle_policy      {Basic} */
#if defined(__H2XML__)
   uint8_t  payload[0];
    /**<
         @h2xmle_description {Payload}
         h2xmle_rangeList   {"For fmt_id = PARAM_ID_PCM_OUTPUT_FORMAT_CFG and data_format=DATA_FORMAT_FIXED_POINT"=0,
                   "For fmt_id = PARAM_ID_ENCODER_OUTPUT_CONFIG and data_format=DATA_FORMAT_RAW_COMPRESSED"=1}
         @h2xmle_policy      {Basic}
         h2xmle_variableArraySize { (payload_size*4+3) / 4}*/
#endif
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct param_id_encoder_output_config_t param_id_encoder_output_config_t;

#ifdef __cplusplus
}
#endif /*__cplusplus*/


#endif /* MEDIA_FMT_API_BASIC_H */
