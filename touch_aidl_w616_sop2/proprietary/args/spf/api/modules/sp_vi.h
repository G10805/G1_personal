#ifndef __SP_VI_H__
#define __SP_VI_H__

/*==============================================================================
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

  @file sp_vi.h
  @brief This file contains APIs for Speaker Protection Module
==============================================================================*/

/*==============================================================================
  Edit History

  when       who        what, where, why
  --------   ---        ------------------------------------------------------
10 feb 2020  smanglam    Speaker Protection
==============================================================================*/

/** @h2xml_title1           {Speaker Protection}
    @h2xml_title_date       {January 7, 2020} */
#include "module_cmn_api.h"
#include "ar_defs.h"
#include "imcl_spm_intent_api.h"

/*------------------------------------------------------------------------------
   Module
------------------------------------------------------------------------------*/
/*==============================================================================
   Constants
==============================================================================*/
#define VI_MAX_INPUT_PORTS 1
#define VI_MAX_OUTPUT_PORTS 1
#define VI_STACK_SIZE 4096

/* Maximum number of stages in the IIR filter. */
#define SP_IIR_TDF2_STAGES_MAX 5

/* Maximum number of samples per packet. */
#define MAX_SAMPLES_IN_PACKET 480

/* Number of numerators per stage in the IIR filter. */
#define SP_IIR_TDF2_NUM_NUM_PER_STAGE 3

/* Number of denominators per stage in the IIR filter. */
#define SP_IIR_TDF2_NUM_DEN_PER_STAGE 2

/* Unique Module ID */
#define MODULE_ID_SPEAKER_PROTECTION_VI 0x0700109E

/** @h2xmlm_module       {"MODULE_ID_SPEAKER_PROTECTION_VI",
                          MODULE_ID_SPEAKER_PROTECTION_VI}
    @h2xmlm_displayName  {"Speaker Protection - VI"}
    @h2xmlm_toolPolicy   {Calibration;CALIBRATION}
    @h2xmlm_description  {ID of the VI module.\n

      This module supports the following parameter IDs:\n
     - #PARAM_ID_SP_VI_OP_MODE_CFG\n
     - #PARAM_ID_SP_TH_VI_R0T0_CFG\n
     - #PARAM_ID_SP_VI_STATIC_CFG\n
     - #PARAM_ID_SP_TH_VI_DYNAMIC_CFG\n
     - #PARAM_ID_SP_TH_VI_CALIB_RES_CFG\n
     - #PARAM_ID_SP_TH_VI_FTM_CFG\n
     - #PARAM_ID_SP_TH_VI_FTM_PARAMS\n
     - #PARAM_ID_SP_TH_VI_V_VALI_CFG\n
     - #PARAM_ID_SP_TH_VI_V_VALI_PARAMS\n
     - #PARAM_ID_SP_EX_VI_DYNAMIC_CFG\n
     - #PARAM_ID_SP_EX_VI_STATS\n
     - #PARAM_ID_SP_EX_VI_MODE_CFG\n
     - #PARAM_ID_SP_EX_VI_FTM_CFG\n
     - #PARAM_ID_SP_EX_VI_FTM_PARAMS\n
     - #PARAM_ID_SP_VI_CHANNEL_MAP_CFG\n
     - #PARAM_ID_VI_OUTPUT_SPLIT_ENABLE\n

It also supports the following events:
     - #EVENT_ID_SP_VI_DC_DETECTION
        The payload of this event includes num channels followed by status of DC presence on each channel

     - #EVENT_ID_VI_CALIBRATION
        It populates and returns the payload corresponding to 'PARAM_ID_SP_TH_VI_CALIB_RES_CFG' param

     - #EVENT_ID_TH_VI_V_VALI_PARAMS
        It populates and returns the payload corresponding to 'PARAM_ID_SP_TH_VI_V_VALI_PARAMS' param

      All parameter IDs are device independent.\n

     Supported Input Media Format:\n
*  - Data Format          : FIXED_POINT\n
*  - fmt_id               : Don't care\n
*  - Sample Rates         : 48000\n
*  - Number of channels   : 2, 4\n
*  - Channel type         : 1 to 63\n
*  - Bits per sample      : 16 , 32 \n
*  - Q format             : 15 for bps = 16 and 27 for bps = 32\n
*  - Interleaving         : de-interleaved unpacked\n
*  - Signed/unsigned      : Signed }

     @h2xmlm_toolPolicy              {Calibration}

    @h2xmlm_dataMaxInputPorts    {VI_MAX_INPUT_PORTS}
     @h2xmlm_dataInputPorts       {IN=2}
     @h2xmlm_dataMaxOutputPorts   {VI_MAX_OUTPUT_PORTS}
     @h2xmlm_dataOutputPorts      {OUT=1}
     @h2xmlm_supportedContTypes  {APM_CONTAINER_TYPE_GC}
     @h2xmlm_isOffloadable       {false}
     @h2xmlm_stackSize            {VI_STACK_SIZE}
    @h2xmlm_ctrlDynamicPortIntent  { "SP VI intent id for communicating Vsens and Isens data" = INTENT_ID_SP, maxPorts=
1 }
     @h2xmlm_ToolPolicy              {Calibration}

    @{                   <-- Start of the Module -->

 @h2xml_Select        {"param_id_module_enable_t"}
 @h2xmlm_InsertParameter
   */

/*------------------------------------------------------------------------------
   Parameters
------------------------------------------------------------------------------*/

/*==============================================================================
   Constants
==============================================================================*/
/* Vsens from the left speaker. */
#define SP_VI_VSENS_LEFT_CHAN 1

/* Isens from the left speaker. */
#define SP_VI_ISENS_LEFT_CHAN 2

/* Vsens from the right speaker. */
#define SP_VI_VSENS_RIGHT_CHAN 3

/* Isens from the right speaker. */
#define SP_VI_ISENS_RIGHT_CHAN 4

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_VI_OP_MODE_CFG 0x080011F4

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_vi_op_mode_cfg_t param_id_sp_vi_op_mode_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_VI_OP_MODE_CFG",
                         PARAM_ID_SP_VI_OP_MODE_CFG}
    @h2xmlp_description {parameter used process Vsens and Isens data for
                         extracting the speaker temperature and excursion.}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_vi_op_mode_cfg_t
{

   uint32_t num_speakers;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   uint32_t th_operation_mode;
   /**< @h2xmle_description {Operation mode of thermal VI module.}
        @h2xmle_rangeList   {"Normal Running mode"=0;
                             "Calibration mode"=1;
                             "Factory Test Mode"=2;
                             "V-Validation Mode"=3} */

   uint32_t th_quick_calib_flag;
   /**< @h2xmle_description {Indicates whether calibration is to be done in quick
                              mode or not. This field is valid only in
                             Calibration mode (operation_mode = 1)}.
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1} */

   uint32_t th_r0t0_selection_flag[0];
   /**< @h2xmle_description {Specifies which set of R0, T0 values the algorithm
                             will use. This field is valid only in Normal mode
                             (operation_mode = 0).}
        @h2xmle_variableArraySize  {num_speakers}
        @h2xmle_rangeList   {"Use calibrated R0, T0 value"=0;
                             "Use safe R0, T0 value"=1} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TH_VI_R0T0_CFG 0x080011F5

/*==============================================================================
   Type definitions
==============================================================================*/

/* R0T0 channel dependent cfg sub structure */
typedef struct vi_r0t0_cfg_t vi_r0t0_cfg_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct vi_r0t0_cfg_t
{
   int32_t r0_cali_q24;
   /**< @h2xmle_description {Calibration point resistance per device (in Ohms).
                             This field is valid only in Normal mode (operation_mode = 0).}
        @h2xmle_range       {33554432..1073741824}
        @h2xmle_default     {33554432}
        @h2xmle_dataFormat  {Q24} */

   int16_t t0_cali_q6;
   /**< @h2xmle_description {Calibration point temperature per device (in degrees C).
                             This field is valid in both Normal mode and Calibration mode.}
        @h2xmle_range       {-1920..5120 }
        @h2xmle_dataFormat  {Q6} */

   int16_t reserved;
   /**< @h2xmle_description {Reserved Field.}
        @h2xmle_visibility  {hide}
        @h2xmle_default     {0} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* Structure definition for Parameter */
typedef struct param_id_sp_th_vi_r0t0_cfg_t param_id_sp_th_vi_r0t0_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_VI_R0T0_CFG",
                         PARAM_ID_SP_TH_VI_R0T0_CFG}
    @h2xmlp_description {parameter used to configure static R0T0 parameters of
                         thermal VI processing..}
    @h2xmlp_toolPolicy  {CALIBRATION} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_vi_r0t0_cfg_t
{

   uint32_t num_speakers;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   vi_r0t0_cfg_t vi_r0t0_cfg[0];
   /**< @h2xmle_description {R0T0 channel dependent cfg sub structure.}
        @h2xmle_variableArraySize  {num_speakers} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_VI_STATIC_CFG 0x080011F6

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_vi_static_cfg_t param_id_sp_vi_static_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_VI_STATIC_CFG",
                         PARAM_ID_SP_VI_STATIC_CFG}
    @h2xmlp_description {parameter used to configure static parameters of
                         thermal VI processing.}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC_READONLY} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_vi_static_cfg_t
{
   uint32_t num_speakers;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   uint32_t sampling_rate;
   /**< @h2xmle_description {Sampling rate of VI signal.}
        @h2xmle_rangeList   {"48kHz"=48000}
        @h2xmle_default     {48000} */

   uint32_t fpilot;
   /**< @h2xmle_description {Pilot tone frequencies.
                             Pilot tone frequency values MUST be the
                      same for RX/TH/EX modules}
        @h2xmle_rangeList   {"16"=16; "40"=40}
        @h2xmle_default     {40} */

   uint32_t th_ctrl_rate;
   /**< @h2xmle_description {Thermal control rate.}
        @h2xmle_rangeList   {"1000hz"=1000}
        @h2xmle_default     {1000} */

   uint32_t th_frame_size_ms;
   /**< @h2xmle_description {Frame size for TH_VI processing.}
        @h2xmle_rangeList   {"1ms"=1}
        @h2xmle_default     {1} */

   int32_t th_pow_supply_freq;
   /**< @h2xmle_description {Specifies the power supply frequency.}
        @h2xmle_rangeList   {"disabled"=0; "50Hz"=50; "60Hz"=60}
        @h2xmle_default     {0} */

   uint32_t th_temp_est_selection_flag;
   /**< @h2xmle_description {Temperature estimation selection flag.
                             Method2 also requires setting of spk_delta_t_delta_r_ratio_u16q9 param.}
        @h2xmle_rangeList   {Method1=0;Method2=1}
        @h2xmle_default     {0} */

   uint32_t ex_frame_size;
   /**< @h2xmle_description {frame size for EX_VI parameter tracking in ms.}
        @h2xmle_rangeList   {"8"=8; "32"=32}
        @h2xmle_default     {8} */

   uint32_t ex_tuning_mode_enable_flag;
   /**< @h2xmle_description {flag for dumping predicted excursion using Vsens at 48kHz.
                             It's only for laser display. Should be disabled at normal mode on devices.}
        @h2xmle_rangeList   {"disable"=0;"enable"=1}
        @h2xmle_default     {0} */

   uint32_t ex_DC_detection_enable_flag;
   /**< @h2xmle_description {flag for DC detection from VIsens.
                             HAL will set a periodic query on the DC presense flag.
                             If DC is present, HAL will shut down WSA to protect spkrs.}
        @h2xmle_rangeList   {"disable"=0;"enable"=1}
        @h2xmle_default     {0} */

   uint32_t ex_warmup_time_ms;
   /**< @h2xmle_description {EXVI parameter tracking will start after this much of warmup time
                             in msec due to inaccurate Rvc estimation at the beginning of playback}
        @h2xmle_range       {100..500}
        @h2xmle_default     {200} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TH_VI_DYNAMIC_CFG 0x080011F7

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_th_vi_dynamic_cfg_t param_id_sp_th_vi_dynamic_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_VI_DYNAMIC_CFG",
                         PARAM_ID_SP_TH_VI_DYNAMIC_CFG}
    @h2xmlp_description {parameter used to configure dynamic thermal parameters
                         for feedback speaker protection VI processing.}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC_READONLY} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_vi_dynamic_cfg_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   uint32_t pilot_tone_scale_u32q18[0];
/**< @h2xmle_description {scale of the pilot tone relative to
                          0 dBFS (for example, -40 dB from the full scale).}
     @h2xmle_variableArraySize  {num_ch}
     @h2xmle_range       {0..65535}
     @h2xmle_dataFormat  {Q18}
     @h2xmle_default     {5230} */

#ifdef __H2XML__
   int32_t r0_safe_q24[0];
   /**< @h2xmle_description {backup calibration point resistance
                             when per-device calibration is not available (in
                             Ohms.)}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {33554432..1073741824}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {104857600} */

   int16_t t0_safe_q6[0];
   /**< @h2xmle_description {backup calibration point temperature
                             when per-device calibration is not available (in
                             degrees C).}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {-1920..5120}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {1600} */

   int16_t r_spk_coil_typ_q8[0];
   /**< @h2xmle_description {nominal AC impedance at 2kHz and rated power of spkr (in Ohms ).}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {512..32767}
        @h2xmle_default     {2048}
        @h2xmle_dataFormat  {Q8} */

   int16_t r_spk_coil_factor_q10[0];
   /**< @h2xmle_description {factor used to determine the threshold
                             for speaker coil resistance (threshold for speaker
                             coil resistance = factor * speaker coil resistance).
                             If (Rvc > factor*(AC impedance)), the speaker is considered
                             open.}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {-32768..32767}
        @h2xmle_dataFormat  {Q10}
        @h2xmle_default     {2048} */

   int16_t v_full_sc_q10[0];
   /**< @h2xmle_description {voltage for digitally full scale
                             signal.}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..32767}
        @h2xmle_dataFormat  {Q10}
        @h2xmle_default     {8192} */

   uint16_t spk_resistivity_coeff_u16q7[0];
   /**< @h2xmle_description {one over temperature coeffcient alpha}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..65535}
        @h2xmle_dataFormat  {Q7}
        @h2xmle_default     {32570} */

   uint16_t vsen_2_v_u16q11[0];
   /**< @h2xmle_description {conversion factor from Vsens digital
                             to volts.}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..65535}
        @h2xmle_dataFormat  {Q11}
        @h2xmle_default     {21466} */

   uint16_t isen_2_a_u16q11[0];
   /**< @h2xmle_description {conversion factor from Isens digital
                             to amps.}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..65535}
        @h2xmle_dataFormat  {Q11}
        @h2xmle_default     {14143} */

   uint16_t v_pilot_thsd_u16q16[0];
   /**< @h2xmle_description {fraction of the expected pilot tone.
                             Below this value, the  resistance update is
                             unreliable.}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..65535}
        @h2xmle_dataFormat  {Q16}
        @h2xmle_default     {26214} */

   int32_t trace_resistance_tx_q24[0];
   /**< @h2xmle_description {trace resistance from speaker to Vsens tap points in Ohm}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {0} */

   int32_t wait_time_ms[0];
   /**< @h2xmle_description {wait time for initial settling before
                             collecting statistics (in milli seconds).}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {100..5000}
        @h2xmle_default     {200} */

   int32_t cali_time_ms[0];
   /**< @h2xmle_description {calibration time  (in milli seconds).}
        @h2xmle_variableArraySize  {num_ch}
            @h2xmle_range       {500..30000}
        @h2xmle_default     {2000} */

   uint16_t r0_und_est_fac_q16[0];
   /**< @h2xmle_description {underestimate R0 if it is quick
                             calibration.}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..65535}
        @h2xmle_dataFormat  {Q16}
        @h2xmle_default     {0} */

   int16_t r0_rng_fac_q10[0];
   /**< @h2xmle_description {factor used to determine the threshold
                             for calculated calibrated resistance (threshold for
                             calibrated resistance = factor * calibrated
                             resistance). After this threshold, calibrated
                             resistance is considered as failed.}
            @h2xmle_variableArraySize  {num_ch}
            @h2xmle_range       {-32768..32767}
            @h2xmle_dataFormat  {Q10}
            @h2xmle_default     {1536} */

   int16_t t0_min_q6[0];
   /**< @h2xmle_description {minimum expected temperature from
        calibration (in degrees C).}
            @h2xmle_variableArraySize  {num_ch}
            @h2xmle_range       {-1920..5120}
            @h2xmle_dataFormat  {Q6}
            @h2xmle_default     {0} */

   int16_t t0_max_q6[0];
   // 12 int 16 params so far, so no holes in struct
   /**< @h2xmle_description {maximum expected temperature from
            calibration (in degrees C).}
            @h2xmle_variableArraySize  {num_ch}
            @h2xmle_range       {-1920..5120}
            @h2xmle_dataFormat  {Q6}
            @h2xmle_default     {3200} */

   uint16_t spk_delta_t_delta_r_ratio_u16q9[0];
   /**< @h2xmle_description {speaker deltaT/deltaR = 1/(change in resistance
                             per unit change in temperature)}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0x0..0xffff}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {0x4286} */

   uint16_t reserved[0];
/**< @h2xmle_description {Reserved}
     @h2xmle_variableArraySize  {num_ch}
     @h2xmle_default     {0} */

#endif //__H2XML__
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TH_VI_CALIB_RES_CFG 0x080011F8

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_th_vi_calib_res_cfg_t param_id_sp_th_vi_calib_res_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_VI_CALIB_RES_CFG",
                         PARAM_ID_SP_TH_VI_CALIB_RES_CFG}
    @h2xmlp_description {Parameter used to get the calibrated resistance value
                         from the feedback speaker VI processing module.}
    @h2xmlp_toolPolicy  {RTC} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_vi_calib_res_cfg_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   uint32_t state;
   /**< @h2xmle_description {Represents the calibration state for both speakers.
                             The state of each speaker is tied together.}
        @h2xmle_rangeList   {"Incorrect operation mode (isn't normal mode)" = 0;
                             "Inactive mode (port is not started)" = 1;
                             "Wait state" = 2;
                             "Calibration in progress state" = 3;
                             "Calibration Success" = 4;
                             "Calibration Failed" = 5} */
   int32_t r0_cali_q24[0];
   /**< @h2xmle_description {Calibration point resistance per device.
                             In Calibration mode, this resistance is not valid.}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {33554432..1073741824}
        @h2xmle_default     {33554432}
        @h2xmle_dataFormat  {Q24} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*------------------------------------------------------------------------------
   Parameters
------------------------------------------------------------------------------*/
/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TH_VI_FTM_CFG 0x080011F9

/*==============================================================================
   Type definitions
==============================================================================*/

/* TH FTM channel dependent cfg sub structure */
typedef struct vi_th_ftm_cfg_t vi_th_ftm_cfg_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct vi_th_ftm_cfg_t
{
   uint32_t wait_time_ms;
   /**< @h2xmle_description {Wait time to heat up the speaker before collecting
                            statistics for FTM mode (in milli seconds).}
       @h2xmle_range       {0..4294967295} */

   uint32_t ftm_time_ms;
   /**< @h2xmle_description {Time period when FTM statistics are collected (in
                             milli seconds).}
       @h2xmle_range       {0..2000} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* Structure definition for Parameter */
typedef struct param_id_sp_th_vi_ftm_cfg_t param_id_sp_th_vi_ftm_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_VI_FTM_CFG",
                         PARAM_ID_SP_TH_VI_FTM_CFG}
    @h2xmlp_description {parameter used to set the FTM configuration in the
                         feedback speaker thermal VI processing module.}
    @h2xmlp_toolPolicy  {CALIBRATION} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_vi_ftm_cfg_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   vi_th_ftm_cfg_t vi_th_ftm_cfg[0];
   /**< @h2xmle_description {TH FTM channel dependent cfg sub structure.}
        @h2xmle_variableArraySize  {num_ch} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*------------------------------------------------------------------------------
   Parameters
------------------------------------------------------------------------------*/
/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TH_VI_FTM_PARAMS 0x080011FA

/*==============================================================================
   Type definitions
==============================================================================*/

/* TH FTM channel dependent get params sub structure */
typedef struct vi_th_ftm_params_t vi_th_ftm_params_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct vi_th_ftm_params_t
{
   int32_t ftm_dc_res_q24;
   /**< @h2xmle_description {DC resistance value (in Ohms).}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24} */

   int32_t ftm_temp_q22;
   /**< @h2xmle_description {Temperature value (in degrees C).}
       @h2xmle_range       {-125829120..2147483647}
        @h2xmle_dataFormat  {Q22} */

   uint32_t status;
   /**< @h2xmle_description {FTM packet status.}
       @h2xmle_rangeList   {"Incorrect operation mode"=0;
                             "Inactive mode (port is not started)"=1;
                             "Wait state"=2;
                             "In progress state"=3;
                             "Success"=4;
                             "Failed"=5} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* Structure definition for Parameter */
typedef struct param_id_sp_th_vi_ftm_params_t param_id_sp_th_vi_ftm_params_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_VI_FTM_PARAMS",
                         PARAM_ID_SP_TH_VI_FTM_PARAMS}
    @h2xmlp_description {parameter used to get the FTM statistics from the
                         Thermal VI processing module.}
    @h2xmlp_toolPolicy  {RTC} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_vi_ftm_params_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */
   vi_th_ftm_params_t vi_th_ftm_params[0];
   /**< @h2xmle_description {TH FTM channel dependent get params sub structure.}
        @h2xmle_variableArraySize  {num_ch} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*------------------------------------------------------------------------------
   Parameters
------------------------------------------------------------------------------*/
/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TH_VI_V_VALI_CFG 0x080011FB

/*==============================================================================
   Type definitions
==============================================================================*/

/* TH V Vali channel dependent set cfg sub structure */
typedef struct vi_th_v_vali_cfg_t vi_th_v_vali_cfg_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct vi_th_v_vali_cfg_t
{
   uint32_t wait_time_ms;
   /**< @h2xmle_description {Wait time to heat up the speaker before collecting
                             statistics for V VALIDATION mode (in milli seconds).}
       @h2xmle_range       {100..1000}
        @h2xmle_default     {500} */

   uint32_t vali_time_ms;
   /**< @h2xmle_description {Time period when V VALIDATION statistics are collected (in
                             milli seconds).}
       @h2xmle_range       {1000..3000}
        @h2xmle_default     {2000} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* Structure definition for Parameter */
typedef struct param_id_sp_th_vi_v_vali_cfg_t param_id_sp_th_vi_v_vali_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_VI_V_VALI_CFG",
                         PARAM_ID_SP_TH_VI_V_VALI_CFG}
    @h2xmlp_description {parameter used to set the V VALIDATION configuration in the
                         feedback speaker thermal VI processing module.}
    @h2xmlp_toolPolicy  {CALIBRATION} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_vi_v_vali_cfg_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   vi_th_v_vali_cfg_t vi_th_v_vali_cfg[0];
   /**< @h2xmle_description {TH V Vali channel dependent set cfg sub structure.}
        @h2xmle_variableArraySize  {num_ch} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*------------------------------------------------------------------------------
   Parameters
------------------------------------------------------------------------------*/
/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TH_VI_V_VALI_PARAMS 0x080011FC

/*==============================================================================
   Type definitions
==============================================================================*/

/* TH V Vali channel dependent get params sub structure */
typedef struct vi_th_v_vali_params_t vi_th_v_vali_params_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct vi_th_v_vali_params_t
{
   uint32_t vrms_q24;
   /**< @h2xmle_description {RMS voltage value}
       @h2xmle_range       {0..33554432}
        @h2xmle_default     {3355443}
        @h2xmle_dataFormat  {Q24} */

   uint32_t status;
   /**< @h2xmle_description {V Validation status.}
       @h2xmle_rangeList   {"Incorrect operation mode"=2;
                             "Inactive mode"=3;
                             "Wait state"=4;
                             "In progress state"=5;
                             "Success"=1;
                             "Failed"=0} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
/* Structure definition for Parameter */
typedef struct param_id_sp_th_vi_v_vali_params_t param_id_sp_th_vi_v_vali_params_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_VI_V_VALI_PARAMS",
                         PARAM_ID_SP_TH_VI_V_VALI_PARAMS}
    @h2xmlp_description {parameter used to get the V VALIDATION statistics from the
                         Thermal VI processing module.}
    @h2xmlp_toolPolicy  {RTC} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_vi_v_vali_params_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   vi_th_v_vali_params_t vi_th_v_vali_params[0];
   /**< @h2xmle_description {TH V Vali channel dependent get params sub structure.}
        @h2xmle_variableArraySize  {num_ch} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_EX_VI_DYNAMIC_CFG 0x080011FD

/*==============================================================================
   Type definitions
==============================================================================*/
/* tdf2 configuration structure for RX_EX Dynamic config. */
typedef struct sp_tdf_2_cfg_t sp_tdf_2_cfg_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct sp_tdf_2_cfg_t
{
   int16_t numCh;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1}
        @h2xmle_default     {1} */

   int16_t stages;
   /**< @h2xmle_description {Number of stages.}
        @h2xmle_range       {1..5}
        @h2xmle_default     {1} */

   int32_t num[SP_IIR_TDF2_STAGES_MAX * SP_IIR_TDF2_NUM_NUM_PER_STAGE];
   /**< @h2xmle_description {Numerator coefficients.}
        @h2xmle_defaultList {1073741824,0,0,
                            0, 0, 0,
                      0, 0, 0,
                      0, 0, 0,
                      0, 0, 0} */

   int32_t den[SP_IIR_TDF2_STAGES_MAX * SP_IIR_TDF2_NUM_DEN_PER_STAGE];
   /**< @h2xmle_description {Denominator coefficients.}
        @h2xmle_defaultList {0, 0,
                            0, 0,
                      0, 0,
                      0, 0,
                      0, 0} */

   int16_t numQ[SP_IIR_TDF2_STAGES_MAX];
   /**< @h2xmle_description {Q factor of numerator coefficients}
        @h2xmle_defaultList {1,0,0,0,0} */

   int16_t denQ[SP_IIR_TDF2_STAGES_MAX];
   /**< @h2xmle_description {Q factor of denominator coefficients.}
        @h2xmle_defaultList {1,0,0,0,0} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* Structure definition for Parameter */
typedef struct param_id_sp_ex_vi_dynamic_cfg_t param_id_sp_ex_vi_dynamic_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_VI_DYNAMIC_CFG",
                         PARAM_ID_SP_EX_VI_DYNAMIC_CFG}
    @h2xmlp_description {parameter used to configure the dynamic parameters of
                         excursion VI processing.}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC_READONLY} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_vi_dynamic_cfg_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   int32_t Re_ohm_q24[0];
/**< @h2xmle_description {DC resistance of voice coil at room temperature or small signal level in Ohm}
     @h2xmle_variableArraySize  {num_ch}
     @h2xmle_range       {33554432..2147483647}
     @h2xmle_dataFormat  {Q24}
     @h2xmle_default     {134217728} */

#ifdef __H2XML__
   int32_t Le_mH_q24[0];
   /**< @h2xmle_description {Voice coil inductance in mH}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {-2147483647..2147483647.}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {671088} */

   int32_t L2_mH_q24[0];
   /**< @h2xmle_description {Para-inductance of voice coil in LR2 inductor model in mH}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {-2147483647..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {33554} */

   int32_t R2_ohm_q24[0];
   /**< @h2xmle_description {Electrical resistance in LR2 inductor model in Ohm}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {-2147483647..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {16777216} */

   int32_t Bl_q24[0];
   /**< @h2xmle_description {Force factor (Bl product)}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {16777216} */

   int32_t Mms_gram_q24[0];
   /**< @h2xmle_description {Mechanical mass of loudspeaker diaphragm in gram}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {2722896} */

   int32_t Rms_KgSec_q24[0];
   /**< @h2xmle_description {Mechanical damping or resistance of loudspeaker in Kg/sec}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {5056765} */

   int32_t Kms_Nmm_q24[0];
   /**< @h2xmle_description {Mechanical stiffness of driver suspension in N/mm}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {68155531} */

   int32_t Fres_Hz_q20[0];
   /**< @h2xmle_description {Resonance frequency in Hz}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q20}
        @h2xmle_default     {838860800} */

   int32_t Qms_q24[0];
   /**< @h2xmle_description {Mechanical Q-factor}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {50331648} */

   int32_t amp_gain_q24[0];
   /**< @h2xmle_description {speaker peak voltage for a digitally full-scale signal
                             Default in decimal: 8
                             Range in decimal: 0 - 128}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {134217728} */

   int32_t vsens_scale_q24[0];
   /**< @h2xmle_description {conversion factor from Vsens digital to volts}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {134217728} */

   int32_t isens_scale_q24[0];
   /**< @h2xmle_description {conversion factor from Isens digital to ampere}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {33554432} */

   int32_t trace_resistance_tx_q24[0];
   /**< @h2xmle_description {trace resistance from speaker to Vsens tap points in Ohm}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {0} */

   int32_t trace_resistance_rx_q24[0];
   /**< @h2xmle_description {trace resistance from amp output to speakers in Ohm}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {0} */

   int32_t DC_pow_thr_mW[0];
   /**< @h2xmle_description {DC power threshold for DC detection in mW}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..4000}
        @h2xmle_dataFormat  {Q0}
        @h2xmle_default     {500} */

   int32_t DC_hold_time_mSec[0];
   /**< @h2xmle_description {If DC power is higher than the threshold for this much of time in msec, DC is present}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_range       {0..5000}
        @h2xmle_dataFormat  {Q0}
        @h2xmle_default     {500} */

   int32_t safety_mode[0];
   /**< @h2xmle_description {flag for conservative adaptation only for DC resistance}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_rangeList   {"disable"=0;"enable"=1}
        @h2xmle_default     {1} */

   sp_tdf_2_cfg_t bandpss_LPfilter_cfg[0];
   /**< @h2xmle_description {LPF config}
        @h2xmle_variableArraySize  {num_ch} */

   sp_tdf_2_cfg_t bandpss_HPfilter_cfg[0];
   /**< @h2xmle_description {HPF config}
        @h2xmle_variableArraySize  {num_ch} */

   sp_tdf_2_cfg_t inductor_filter_cfg[0];
/**< @h2xmle_description {Inductor filter config}
     @h2xmle_variableArraySize  {num_ch} */

#endif //__H2XML__
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_EX_VI_STATS 0x080011FE

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_ex_vi_stats_t param_id_sp_ex_vi_stats_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_VI_STATS",
                         PARAM_ID_SP_EX_VI_STATS}
    @h2xmlp_description {Parameter used to log excursion VI algorithm internal
                         variables.}
    @h2xmlp_toolPolicy  {CALIBRATION} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_vi_stats_t
{
   uint32_t frame_duration_in_ms;
   /**< @h2xmle_description {Frame duration in milliseconds for Excursion VI
                             statistics.} */

   uint32_t frame_num;
   /**< @h2xmle_description {Frame number of the Excursion VI statistics. This
                             field is reset to zero after crossing the maximum
                             limit for uint32 variables.} */

   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   int32_t xpred_from_vsens[0][MAX_SAMPLES_IN_PACKET];
   /**< @h2xmle_description {xpred from vsens.} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_EX_VI_MODE_CFG 0x080011FF

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_ex_vi_mode_cfg_t param_id_sp_ex_vi_mode_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_VI_MODE_CFG",
                         PARAM_ID_SP_EX_VI_MODE_CFG}
    @h2xmlp_description {parameter used to configure the mode of Feedback
                         Speaker Protection Excursion VI processing.}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_vi_mode_cfg_t
{
   uint32_t operation_mode;
   /**< @h2xmle_description {Operation mode of the Excursion VI module.}
        @h2xmle_rangeList   {"Normal Running mode"=0;
                             "FTM mode"=1} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_EX_VI_FTM_CFG 0x08001200

/*==============================================================================
   Type definitions
==============================================================================*/

/* EX FTM channel dependent set cfg sub structure */
typedef struct vi_ex_ftm_cfg_t vi_ex_ftm_cfg_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct vi_ex_ftm_cfg_t
{
   uint32_t wait_time_ms;
   /**< @h2xmle_description {Wait time to heat up the speaker before collecting
                             statistics for FTM mode (in milli seconds).}
        @h2xmle_range       {0..4294967295} */

   uint32_t ftm_time_ms;
   /**< @h2xmle_description {Period of time when FTM statistics are collected (in
                             milli seconds).}
        @h2xmle_range       {0..2000} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
/* Structure definition for Parameter */
typedef struct param_id_sp_ex_vi_ftm_cfg_t param_id_sp_ex_vi_ftm_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_VI_FTM_CFG",
                         PARAM_ID_SP_EX_VI_FTM_CFG}
    @h2xmlp_description {parameter used to set the FTM configuration in the
                         Excursion VI processing module.}
    @h2xmlp_toolPolicy  {CALIBRATION} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_vi_ftm_cfg_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   vi_ex_ftm_cfg_t vi_ex_ftm_cfg[0];
   /**< @h2xmle_description {EX FTM channel dependent set cfg sub structure.}
        @h2xmle_variableArraySize  {num_ch} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_EX_VI_FTM_PARAMS 0x08001201

/*==============================================================================
   Type definitions
==============================================================================*/

/* EX FTM channel dependent get params sub structure */
typedef struct vi_ex_ftm_params_t vi_ex_ftm_params_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct vi_ex_ftm_params_t
{
   int32_t ftm_Re_q24;
   /**< @h2xmle_description {DC resistance of voice coil at room temperature or small signal level in Ohm}
        @h2xmle_dataFormat  {Q24} */

   int32_t ftm_Bl_q24;
   /**< @h2xmle_description {Force factor (Bl product)}
        @h2xmle_dataFormat  {Q24} */

   int32_t ftm_Rms_q24;
   /**< @h2xmle_description {Mechanical damping or resistance of loudspeaker in Kg/sec}
        @h2xmle_dataFormat  {Q24} */

   int32_t ftm_Kms_q24;
   /**< @h2xmle_description {Mechanical stiffness of driver suspension in N/mm}
        @h2xmle_dataFormat  {Q24} */

   int32_t ftm_Fres_q20;
   /**< @h2xmle_description {Resonance frequency in Hz}
        @h2xmle_dataFormat  {Q20} */

   int32_t ftm_Qms_q24;
   /**< @h2xmle_description {Mechanical Q-factor}
        @h2xmle_dataFormat  {Q24} */

   uint32_t status;
   /**< @h2xmle_description {FTM packet status.}
       @h2xmle_rangeList   {"Incorrect operation mode"=0;
                             "Inactive mode (port is not started)"=1;
                             "Wait state"=2;
                             "In progress state"=3;
                             "Success"=4;
                             "Failed"=5} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
/* Structure definition for Parameter */
typedef struct param_id_sp_ex_vi_ftm_params_t param_id_sp_ex_vi_ftm_params_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_VI_FTM_PARAMS",
                         PARAM_ID_SP_EX_VI_FTM_PARAMS}
    @h2xmlp_description {parameter used to get the FTM statistics from the
                         Excursion VI module.}
    @h2xmlp_toolPolicy  {RTC} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_vi_ftm_params_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   vi_ex_ftm_params_t vi_ex_ftm_params[0];
   /**< @h2xmle_description {EX FTM channel dependent get params sub structure.}
        @h2xmle_variableArraySize  {num_ch} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_VI_CHANNEL_MAP_CFG 0x08001203

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_vi_channel_map_cfg_t param_id_sp_vi_channel_map_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_VI_CHANNEL_MAP_CFG",
                         PARAM_ID_SP_VI_CHANNEL_MAP_CFG}
    @h2xmlp_description {parameter used to set the input channel mapping for Vsens and Isens
                         for the VI module.}
    @h2xmlp_toolPolicy  {CALIBRATION} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_vi_channel_map_cfg_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Tx signal.}
        @h2xmle_rangeList   {"Two" = 2; "Four" = 4}
        @h2xmle_default     {2} */

   int32_t chan_info[0];
   /**< @h2xmle_description {Channel mapping array that provides information
                             on the order in which the Vsens and Isens of
                             different speakers come into a Tx port.
                             - SP_VI_VSENS_LEFT_CHAN(1)
                             - SP_VI_ISENS_LEFT_CHAN(2)
                             - SP_VI_VSENS_RIGHT_CHAN(3)
                             - SP_VI_ISENS_RIGHT_CHAN(4)
                      Some possible configurations are (1,2,3,4), (4,2,1,3),
                      (1,2,0,0),(4,3,0,0), and (2,1,0,0).
                      All channels must be unique. The order does not matter
                      as long as the channels are valid.
                      If only two channels are sent (V and I), they must be
                             first two channels: Eg: (4,3), (1,2), etc.}
       @h2xmle_variableArraySize  {num_ch} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/
// This param is introduced to allow client to choose whether he wants the output of
// VI module with bits per sample as 32 (Vsense and Isense data merged into single
// channel for each corresponding channel on RX path) or as 16. Valid only if input has bps = 32

#define PARAM_ID_VI_OUTPUT_SPLIT_ENABLE 0x080011C2

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_vi_output_bps_split_enable_t param_id_vi_output_bps_split_enable_t;

/** @h2xmlp_parameter   {"PARAM_ID_VI_OUTPUT_SPLIT_ENABLE",
                         PARAM_ID_VI_OUTPUT_SPLIT_ENABLE}
    @h2xmlp_description {Parameter used to indicate if the 32bit(per channel) output of VI
    module should be split into two channels of 16 bits output. This is valid only if the
    input to VI module has bps =32.}
    @h2xmlp_toolPolicy  {CALIBRATION} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_vi_output_bps_split_enable_t
{
   uint32_t bps_split_enable;
   /**< @h2xmle_description {Bits per sample of the vi module output. If the input
      has bits per sample as 32 and this is enabled, it splits the output into
      two channels with bits per sample as 16 for each corresponding input channel.}
        @h2xmle_rangeList   {"ENABLE" = 1; "DISABLE" = 0}
        @h2xmle_default     {0} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

#define EVENT_ID_SP_VI_DC_DETECTION 0x0800119C
/*==============================================================================
   Type definitions
==============================================================================*/

/** @h2xmlp_parameter   {"EVENT_ID_SP_VI_DC_DETECTION",
                          EVENT_ID_SP_VI_DC_DETECTION}
    @h2xmlp_description { Direct Current Detection event raised by the Speaker Protection Module.}
    @h2xmlp_toolPolicy  { NO_SUPPORT}*/

#define EVENT_ID_VI_CALIBRATION 0x0800119F
/*==============================================================================
   Type definitions
==============================================================================*/

/** @h2xmlp_parameter   {"EVENT_ID_VI_CALIBRATION",
                          EVENT_ID_VI_CALIBRATION}
    @h2xmlp_description { VI calibration event raised by the Speaker Protection Module.}
    @h2xmlp_toolPolicy  { NO_SUPPORT}*/

#define EVENT_ID_TH_VI_V_VALI_PARAMS 0x08001129
/*==============================================================================
   Type definitions
==============================================================================*/

/** @h2xmlp_parameter   {"EVENT_ID_TH_VI_V_VALI_PARAMS",
                          EVENT_ID_TH_VI_V_VALI_PARAMS}
    @h2xmlp_description { V Validation event raised by the Speaker Protection Module.}
    @h2xmlp_toolPolicy  { NO_SUPPORT}*/

/** @} <-- End of the Module --> */

#endif /* __SP_VI_H__ */
