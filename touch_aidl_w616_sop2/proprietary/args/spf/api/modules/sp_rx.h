#ifndef __SP_RX_H__
#define __SP_RX_H__

/*==============================================================================
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

  @file sp_rx.h
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
#define SP_MAX_INPUT_PORTS 1
#define SP_MAX_OUTPUT_PORTS 1
#define SP_STACK_SIZE 4096

/* Unique Module ID */
#define MODULE_ID_SPEAKER_PROTECTION 0x0700109D

/** @h2xmlm_module       {"MODULE_ID_SPEAKER_PROTECTION",
                          MODULE_ID_SPEAKER_PROTECTION}
    @h2xmlm_displayName  {"Speaker Protection"}
    @h2xmlm_toolPolicy   {Calibration;CALIBRATION}
    @h2xmlm_description  {ID of the Speaker Protection module.\n

      This module supports the following parameter IDs:\n
     - #PARAM_ID_MODULE_ENABLE \n
     - #PARAM_ID_SP_STATIC_CFG\n
     - #PARAM_ID_SP_OP_MODE\n
     - #PARAM_ID_SP_TRANSITION_ASP_ENABLE\n
     - #PARAM_ID_SP_TRANSITION_ASP_CALIB_PARAM\n
     - #PARAM_ID_SP_DYNAMIC_CFG\n
     - #PARAM_ID_SP_TH_DEMO_CFG\n
     - #PARAM_ID_SP_EX_DEMO_CFG\n
     - #PARAM_ID_SP_TH_STATS\n
     - #PARAM_ID_SP_EX_STATS\n
     - #PARAM_ID_SP_STATS\n
     - #PARAM_ID_SP_TMAX_XMAX_LOGGING\n
     - #PARAM_ID_SP_VERSION\n


      All parameter IDs are device independent.\n

     Supported Input Media Format:\n
*  - Data Format          : FIXED_POINT\n
*  - fmt_id               : Don't care\n
*  - Sample Rates         : 48000\n
*  - Number of channels   : 1 and 2\n
*  - Channel type         : 1 to 63\n
*  - Bits per sample      : 16, 24  \n
*  - Q format             : 15 for bps = 16 and 27 for bps = 24\n
supported bps
*  - Interleaving         : de-interleaved unpacked\n
*  - Signed/unsigned      : Signed }

     @h2xmlm_toolPolicy              {Calibration}

    @h2xmlm_dataMaxInputPorts    {SP_MAX_INPUT_PORTS}
     @h2xmlm_dataInputPorts       {IN=2}
     @h2xmlm_dataMaxOutputPorts   {SP_MAX_OUTPUT_PORTS}
     @h2xmlm_dataOutputPorts      {OUT=1}
     @h2xmlm_supportedContTypes  {APM_CONTAINER_TYPE_GC}
     @h2xmlm_isOffloadable        {false}
     @h2xmlm_stackSize            {SP_STACK_SIZE}
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

/* Maximum number of speakers supported in speaker protection Rx
    processing. */
#define SP_NUM_MAX_SPKRS 2

/* Maximum number of stages in the notch filter. */
#define SP_NCH_FILT_STAGES_MAX 5

/* Number of plotting samples in one thermal Rx output packet. */
#define SP_TH_RX_DEMO_SMPL_PER_PKT 10

/* Number of plotting samples in one excursion Rx output packet. */
#define SP_EX_RX_DEMO_SMPL_PER_PKT 10

/* Maximum number of stages in the IIR filter. */
#define SP_IIR_TDF2_STAGES_MAX 5

/* Number of numerators per stage in the IIR filter. */
#define SP_IIR_TDF2_NUM_NUM_PER_STAGE 3

/* Number of denominators per stage in the IIR filter. */
#define SP_IIR_TDF2_NUM_DEN_PER_STAGE 2

/* Maximum number of samples per packet. */
#define MAX_SAMPLES_IN_PACKET 480

/* Max number of vbatt resistance table */
#define SP_NVBATT_DISCRETE 9

/* Max number of die temp resistance table */
#define SP_NDTEMP_DISCRETE 14

/* LPASS CPS HW INTF TYPE*/
#define SOUNDWIRE_MODE 1

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_STATIC_CFG 0x080011E8

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_static_cfg_t param_id_sp_static_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_STATIC_CFG",
                         PARAM_ID_SP_STATIC_CFG}
    @h2xmlp_description {parameter used to configure the static configuration of
                         speaker protection processing.}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC_READONLY}*/
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_static_cfg_t
{
   uint32_t sampling_rate;
   /**< @h2xmle_description {Sampling rate of Rx signal.}
        @h2xmle_rangeList   {"48kHz"=48000}
        @h2xmle_default     {48000} */

   uint32_t bits_per_sample;
   /**< @h2xmle_description {Bits per sample for Rx signal.}
        @h2xmle_rangeList   {"16"=16;"24"=24}
        @h2xmle_default     {16} */

   uint32_t num_speakers;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   uint32_t frame_size_ms;
   /**< @h2xmle_description {Frame size for Rx processing.}
        @h2xmle_rangeList   {"1ms"=1}
        @h2xmle_default     {1} */

   uint32_t features;
   /**< @h2xmle_description {Features enabled in the speaker protection
                             algorithm.}
        @h2xmle_range       {0..0x1f}

        @h2xmle_bitField    {0x00000001}
        @h2xmle_bitName     {"Notch high-pass filter"}
        @h2xmle_description {Notch high-pass filter}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {1}
        @h2xmle_group       {HPF}
        @h2xmle_bitFieldEnd

        @h2xmle_bitField    {0x00000002}
        @h2xmle_bitName     {"Thermal protection"}
        @h2xmle_description {Thermal protection}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {1}
        @h2xmle_bitFieldEnd

        @h2xmle_bitField    {0x00000004}
        @h2xmle_bitName     {"Feedback excursion control"}
        @h2xmle_description {Feedback excursion control}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {1}
        @h2xmle_group       {Exc_Control}
        @h2xmle_bitFieldEnd

        @h2xmle_bitField    {0x00000008}
        @h2xmle_bitName     {"Reserved bits"}
        @h2xmle_description {any bit field from 0x00000008 to 0xfffffff8 is reserved for future use.}
        @h2xmle_default     {0}
        @h2xmle_visibility  {hide}
        @h2xmle_readOnly    {true}
        @h2xmle_bitFieldEnd */

   uint32_t tuning_mode_en_flag;
   /**< @h2xmle_description {Specifies whether RTM tuning mode is enabled.}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {0} */

   uint32_t ctrl_rate;
   /**< @h2xmle_description {Thermal control rate in Hz.(Default: 1000Hz.
                             Indicates 1000 updates per second)}
        @h2xmle_rangeList   {"[1000hz]"=1000}
        @h2xmle_default     {1000} */

   uint32_t num_nch_filter_stages;
   /**< @h2xmle_description {Number of notch filter stages.}
        @h2xmle_range       {0..SP_NCH_FILT_STAGES_MAX}
        @h2xmle_default     {2} */

   int16_t temp_loop_dly_ms;
   /**< @h2xmle_description {Loop delay from the predicted temperature to the
                             temperature information coming back.}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {120} */

   int16_t pow_est_smooth_flag;
   /**< @h2xmle_description {Specifies whether to use IIR filtering to smooth the
                             power estimation.}
        @h2xmle_rangeList   {"Do not use IIR filtering"=0;
                             "Use IIR filtering"=1}
        @h2xmle_default     {1} */

   uint16_t pt_lvl_switch_en;
   /**< @h2xmle_description {Specifies whether to enable the dynamic pilot tone
                             level switch.}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {0} */

   uint16_t pt_masking_delay_ms;
   /**< @h2xmle_description {Delay in ramping down the pilot tone.}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {13} */

   int32_t pt_masking_thr_q27;
   /**< @h2xmle_description {Specifies the input level threshold below which the
                             pilot tone is disabled.}
        @h2xmle_range       {0..134217727}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {189813} */

   int32_t hist_buf_length_ms;
   /**< @h2xmle_description {history buffer length in msec for excursion and power control}
        @h2xmle_range       {4..12}
        @h2xmle_default     {8} */

   int32_t rx_static_gain_q27;
   /**< @h2xmle_description {Gain to be applied to RX signal before speaker protection in linear scale.
                             Default in decimal: 1
                             Range in decimal 0.125 - 3.981}
        @h2xmle_range       {33713968..534330399}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {134217728} */

   int32_t dyn_proc_delay_ms;
   /**< @h2xmle_description {delay in dynamic processing in msec to compensate analysis filter delay}
        @h2xmle_range       {0..10} ms
        @h2xmle_default     {0} */

   uint32_t log_dump_en_flag;
   /**< @h2xmle_description {Flag to enable/disable logging maximum excursion and temperature data.
                             PARAM_ID_SP_TMAX_XMAX_LOGGING param id can log data only if enable = 1.}
        @h2xmle_rangeList   {"disable"=0;"enable"=1}
        @h2xmle_default     {0} */

   uint32_t logging_count_period;
   /**< @h2xmle_description {Duration of monitoring Xmax/Tmax to increase counter in msec}
        @h2xmle_range       {0..0xFFFFFFFF}
        @h2xmle_default     {2000} */

   uint32_t fpilot;
   /**< @h2xmle_description {Pilot tone frequencies(in Hz).
                             Pilot tone frequency values MUST be the same for the RX/TH/EX modules}
        @h2xmle_rangeList   {"16"=16; "40"=40}
        @h2xmle_default     {40} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
     Constants
  ==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_CPS_STATIC_CFG 0x08001258

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_cps_static_cfg_t param_id_sp_cps_static_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_CPS_STATIC_CFG",
                         PARAM_ID_SP_CPS_STATIC_CFG}
    @h2xmlp_description {parameter used to configure the static cps configuration of
                         speaker protection processing.}
    @h2xmlp_toolPolicy  {Calibration;RTC_READONLY} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_cps_static_cfg_t
{
   uint16_t limiter_cps_en_flag;
   /**< @h2xmle_description {Flag to enable/disable CPS limiter based on Vbatt/DieTemp}
     @h2xmle_rangeList   {"disabled"=0; "enabled"=1}
      @h2xmle_default     {0} */

   uint16_t limiter_cps_smooth_VbDT_en_flag;
   /**< @h2xmle_description {Flag that activates the smoothing operator on the CPS limiter applied to Vbatt/DieTemp}
     @h2xmle_rangeList   {"disabled"=0; "enabled"=1}
     @h2xmle_default     {0} */

   int32_t limiter_cps_margin_dB_q15;
   /**< @h2xmle_description {margin in dB about the limiter gain based on Vbatt/DieTemp.
                          Range from -3dB to 3dB}
     @h2xmle_dataFormat  {Q15}
     @h2xmle_default     {3277} */

   int32_t FourOhmTable_GaindB_q24[SP_NDTEMP_DISCRETE * SP_NVBATT_DISCRETE];
   /**< @h2xmle_description {dB limiter gain based on Vbatt/DieTemp.}
       @h2xmle_range       {0..300000000}
        @h2xmle_dataFormat  {Q24}
           @h2xmlx_expandArray  {true}
           @h2xmle_defaultFile  {sp_fourOhmGain.bin}
           @h2xmle_policy  {Advanced} */

   int32_t SixOhmTable_GaindB_q24[SP_NDTEMP_DISCRETE * SP_NVBATT_DISCRETE];
   /**< @h2xmle_description {dB limiter gain based on Vbatt/DieTemp.}
       @h2xmle_range       {0..300000000}
        @h2xmle_dataFormat  {Q24}
           @h2xmlx_expandArray  {true}
           @h2xmle_defaultFile  {sp_sixOhmGain.bin}
           @h2xmle_policy  {Advanced} */

   int32_t EightOhmTable_GaindB_q24[SP_NDTEMP_DISCRETE * SP_NVBATT_DISCRETE];
   /**< @h2xmle_description {dB limiter gain based on Vbatt/DieTemp.}
       @h2xmle_range       {0..300000000}
        @h2xmle_dataFormat  {Q24}
           @h2xmlx_expandArray  {true}
           @h2xmle_defaultFile  {sp_eightOhmGain.bin}
           @h2xmle_policy  {Advanced} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
     Constants
  ==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_CPS_LPASS_HW_INTF_CFG 0x08001259 // we get hardware register addresses

/*==============================================================================
   Type definitions
==============================================================================*/
#define CPS_NUM_VBATT_THRESHOLD_VALUES 3

/* LPASS SWR HW register speaker dependent sub structure payload */

typedef struct pkd_reg_addr_t pkd_reg_addr_t;
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct pkd_reg_addr_t
{
   uint32_t vbatt_pkd_reg_addr;
   /**< @h2xmle_description {Per speaker packed register address written to LPASS RD CMD register.
                                To read Voltage value} */

   uint32_t temp_pkd_reg_addr;
   /**< @h2xmle_description {Per speaker packed register address written to LPASS RD CMD register.
                                To read Temp value} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* LPASS SWR HW register structure payload */
typedef struct lpass_swr_hw_reg_cfg_t lpass_swr_hw_reg_cfg_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct lpass_swr_hw_reg_cfg_t
{
   uint32_t num_spkr;
   /**< @h2xmle_description {Number of Rx speakers.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   uint32_t lpass_wr_cmd_reg_phy_addr;
   /**< @h2xmle_description {LPASS HW register physical address. Threshold values to be written based on vbatt value
      read. Write only at transitions from either of the following values: lower threshold 1, lower threshold 2 and
      normal} */

   uint32_t lpass_rd_cmd_reg_phy_addr;
   /**< @h2xmle_description {LPASS HW register physical address to write packed soundwire register address
                             values of temp and vbatt} */

   uint32_t lpass_rd_fifo_reg_phy_addr;
   /**< @h2xmle_description {LPASS HW register physical address to read temp and vbatt soundwire register values.} */

   pkd_reg_addr_t pkd_reg_addr[0];
   /**< @h2xmle_description {Per speaker packed register address written to LPASS RD CMD register.
                                To read Voltage/Temp value} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* Structure definition for Parameter */
typedef struct param_id_cps_lpass_hw_intf_cfg_t param_id_cps_lpass_hw_intf_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_CPS_LPASS_HW_INTF_CFG",
                         PARAM_ID_CPS_LPASS_HW_INTF_CFG}
    @h2xmlp_description {Parameter used for querying the hardware register details from HLOS. Temp and Voltage}
    @h2xmlp_toolPolicy  {NO_SUPPORT} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_cps_lpass_hw_intf_cfg_t
{
   uint32_t lpass_hw_intf_cfg_mode;
   /**< @h2xmle_description {LPASS HW interface configuration mode.
                             1 - SOUNDWIRE MODE}
        @h2xmle_rangeList   {"SOUNDWIRE"=1}
        @h2xmle_default     {1}  */

   /* Variable payload depending on lpass_hw_intf_cfg_mode will follow here */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
     Constants
  ==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_CPS_LPASS_SWR_THRESHOLDS_CFG 0x08001254 // we get hardware register addresses

/*==============================================================================
   Type definitions
==============================================================================*/

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct cps_reg_wr_values_t
{
   uint32_t value_normal_threshold[CPS_NUM_VBATT_THRESHOLD_VALUES];
   /**< @h2xmle_description {Values to be written to LPASS WR CMD register when vbatt value returns back to
                             normal threshold. vbatt greater than or equal to vbatt_lower_threshold_1} */

   uint32_t value_lower_threshold_1[CPS_NUM_VBATT_THRESHOLD_VALUES];
   /**< @h2xmle_description {Values to be written to LPASS WR CMD register, if vbatt is greater than
                             or equal to vbatt_lower_threshold_2 and if vbatt is less than vbatt_lower_threshold_1} */

   uint32_t value_lower_threshold_2[CPS_NUM_VBATT_THRESHOLD_VALUES];
   /**< @h2xmle_description {Values to be written to LPASS WR CMD register, if vbatt is less than
    *                        vbatt_lower_threshold_2} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

typedef struct param_id_cps_lpass_swr_thresholds_cfg_t param_id_cps_lpass_swr_thresholds_cfg_t;

typedef struct cps_reg_wr_values_t cps_reg_wr_values_t;

/** @h2xmlp_parameter   {"PARAM_ID_CPS_LPASS_SWR_THRESHOLDS_CFG",
                           PARAM_ID_CPS_LPASS_SWR_THRESHOLDS_CFG}
      @h2xmlp_description {Parameter used to configure Vbatt thresholds  values and values to be updated into
   soundwire(SWR). The parameters “vbatt_lower_threshold_1” and “vbatt_lower_threshold_2” in this structure are the
   standard values against which vbatt values are compared and are checked if they lie in “normal_threshold”  range  or
   “lower_threshold_1” range or “lower_threshold_2” range. }
      @h2xmlp_toolPolicy  {CALIBRATION, RTC_READONLY} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_cps_lpass_swr_thresholds_cfg_t
{
   uint32_t num_spkr;
   /**< @h2xmle_description {Number of Rx speakers.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   uint32_t vbatt_lower_threshold_1;
   /**< @h2xmle_description {vbatt lower threshold 1 value. Size = 8bits.
                             if vbatt is greater than or equal to value_lower_threshold_2 and vbatt is less than
                             vbatt_lower_threshold_1, write configured values to LPASS WR CMD register}
        @h2xmle_range       {0..255} */

   uint32_t vbatt_lower_threshold_2;
   /**< @h2xmle_description {vbatt lower threshold 2 value. Size = 8bits.
                             If vbatt is less than vbatt_lower_threshold_2, write configured values to LPASS WR CMD
                             register}
        @h2xmle_range       {0..255} */

   cps_reg_wr_values_t cps_reg_wr_values[0];
   /**< @h2xmle_description {Structures for Vbat values} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Parameter id */
#define PARAM_ID_SP_EX_CPS_DEMO_CFG 0x0800125A

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_ex_cps_demo_cfg_t param_id_sp_ex_cps_demo_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_CPS_DEMO_CFG",
                         PARAM_ID_SP_EX_CPS_DEMO_CFG}
    @h2xmlp_description {parameter used for setting the algorithm internal
                         variables related to ex cps demo rtm configuration.}
    @h2xmlp_toolPolicy  {Calibration} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_cps_demo_cfg_t
{
   uint16_t dsr;
   /**< @h2xmle_description {downsample ratio from internal values to output.}
        @h2xmle_default     {10}  */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_EX_CPS_STATS 0x0800125B

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_ex_cps_stats_t param_id_sp_ex_cps_stats_t;
/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_CPS_STATS",
                         PARAM_ID_SP_EX_CPS_STATS}
    @h2xmlp_description {parameter used for RTM of the algorithm internal
                         variables related to ex cps processing.}
    @h2xmlp_toolPolicy  {NO_SUPPORT} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_cps_stats_t
{
   uint32_t frame_duration_in_ms;
   /**< @h2xmle_description {Frame duration in milliseconds for Excursion cps Rx
                             statistics.} */

   uint32_t frame_num;
   /**< @h2xmle_description {Frame number of the Excursion Rx statistics.} */

   uint32_t num_spkr;
   /**< @h2xmle_description {Number of Rx speakers.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   int32_t dieTemp_rt_q20[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Die temperature of the coil.}
        @h2xmle_dataFormat  {Q20}
        @h2xmle_variableArraySize  {num_spkr} */

   int32_t vbatt_rt_q24[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Battery voltage generated}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_variableArraySize  {num_spkr} */

   int32_t dB_cps_Gain_rt_q24[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {CPS gain in Q24 format.}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_variableArraySize  {num_spkr} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_OP_MODE 0x080011E9

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_op_mode_t param_id_sp_op_mode_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_OP_MODE",
                         PARAM_ID_SP_OP_MODE}
    @h2xmlp_description {parameter used to configure the mode of feedback
                         speaker protection processing}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_op_mode_t
{
   uint32_t operation_mode;
   /**< @h2xmle_description {Operation mode of thermal VI module.}
        @h2xmle_rangeList   {"Normal Running mode" = 0;
                             "Calibration mode"    = 1;
                             "Factory Test Mode"   = 2;
                             "V-Validation Mode"   = 3} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TRANSITION_ASP_ENABLE 0x080011EA

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_transition_asp_enable_t param_id_sp_transition_asp_enable_t;
/** @h2xmlp_parameter   {"PARAM_ID_SP_TRANSITION_ASP_ENABLE",
                         PARAM_ID_SP_TRANSITION_ASP_ENABLE}
    @h2xmlp_description {parameter used to enable transition acoustic shock protection
                         feature.}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC_READONLY} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_transition_asp_enable_t
{
   uint32_t transition_asp_enable_flag;
   /**< @h2xmle_description {Enable/Disable the Transition Acoustic Shock Protection feature.}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {0}
   */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TRANSITION_ASP_CALIB_PARAM 0x080011EB

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_transition_asp_calib_param_t param_id_sp_transition_asp_calib_param_t;
/** @h2xmlp_parameter   {"PARAM_ID_SP_TRANSITION_ASP_CALIB_PARAM",
                         PARAM_ID_SP_TRANSITION_ASP_CALIB_PARAM}
    @h2xmlp_description {parameter used for logging the algorithm internal
                         variables.}
    @h2xmlp_toolPolicy  {CALIBRATION; RTC_READONLY} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_transition_asp_calib_param_t
{
   uint32_t initial_target_gain;
   /**< @h2xmle_description {Initial linear target gain in Q28
                             default: 7.199 (in decimal)
                             Range: 0 - 7.999 (in decimal)}
        @h2xmle_range       {0..2147483647}
        @h2xmle_default     {1932735283}
        @h2xmle_dataFormat  {Q28} */

   uint32_t initial_target_gain_period_in_msec;
   /**< @h2xmle_description {Time period in which target gain remain still as initial target gain in millisecond
      (Q0)}
        @h2xmle_range       {0..2147483647}
        @h2xmle_default     {2000}
        @h2xmle_dataFormat  {Q0} */

   uint32_t rampup_transition_time_in_msec;
   /**< @h2xmle_description {Ramp up transition time from initial linear gain to gain 0dB in millisecond (Q0)}
        @h2xmle_range       {0..2147483647}
        @h2xmle_default     {1000}
        @h2xmle_dataFormat  {Q0} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_DYNAMIC_CFG 0x080011EC

/*==============================================================================
   Type definitions
==============================================================================*/

/* Notch Dyanmic Config structure payload */
typedef struct sp_notch_dynamic_cfg_t sp_notch_dynamic_cfg_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct sp_notch_dynamic_cfg_t
{
   int32_t num[SP_NCH_FILT_STAGES_MAX * SP_IIR_TDF2_NUM_NUM_PER_STAGE];
   /**< @h2xmle_description {Numerator coefficients for notch filter
                             configuration.}
        @h2xmle_defaultList {1055533754, -2111067576, 1055533762,
                            1073741824, -2147483582, 1073741815,
                      0, 0, 0,
                      0, 0, 0,
                      0, 0, 0} */

   int32_t den[SP_NCH_FILT_STAGES_MAX * SP_IIR_TDF2_NUM_DEN_PER_STAGE];
   /**< @h2xmle_description {Denominator coefficients for notch filter
                             configuration.}
        @h2xmle_defaultList {-2121642205, 1048082161,
                            -2136597088, 1063038326,
                             0, 0,
                      0, 0,
                      0, 0} */

   int16_t num_q[SP_NCH_FILT_STAGES_MAX];
   /**< @h2xmle_description {Q factor of numerator coefficients for notch filter
                             configuration.}
        @h2xmle_defaultList {1, 1, 0, 0, 0} */

   int16_t den_q[SP_NCH_FILT_STAGES_MAX];
   /**< @h2xmle_description {Q factor of denominator coefficients for notch
                             filter configuration.}
        @h2xmle_defaultList {1, 1, 0, 0, 0} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/* tdf2 configuration structure for RX_EX Dynamic config. */
typedef struct sp_tdf2_cfg_t sp_tdf2_cfg_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct sp_tdf2_cfg_t
{
   int16_t numCh;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
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
typedef struct param_id_sp_dynamic_cfg_t param_id_sp_dynamic_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_DYNAMIC_CFG",
                         PARAM_ID_SP_DYNAMIC_CFG}
    @h2xmlp_description {Parameter used to configure dynamic parameters for
                         Feedback Speaker Protection module processing}
   @h2xmlp_toolPolicy  {Calibration;RTC_READONLY} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_dynamic_cfg_t
{
   sp_notch_dynamic_cfg_t rx_notch_filter_cfg;
   /**< @h2xmle_description {Payload to configure dynamic
                             notch filter configuration} */

   uint32_t speakers_link_mode;
   /**< @h2xmle_description {Specifies whether the gains applied to the two
                             speakers are linked.}
        @h2xmle_rangeList   {"Unlinked"=0; "Linked"=1}
        @h2xmle_default     {0} */

   uint32_t temp_control_mthd;
   /**< @h2xmle_description {Temperature control method.}
        @h2xmle_rangeList   {"Use feedback temperature only"=0;
                             "Feedback with low pass"=1;
                             "Use feedforward estimated temperature"=2}
        @h2xmle_default     {2} */

   int32_t temp_nominal_q6;
   /**< @h2xmle_description {Assumed voice coil temperature at startup in q6 format.
                             (50 in degree C)}
        @h2xmle_range       {-1920..12800}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {3200} */

   int32_t temp_min_q6;
   /**< @h2xmle_description {Minimum reasonable measured temperature. A
                             temperature lower than this value is ignored. In q6 format.
                             (-30 degree C)}
        @h2xmle_range       {-1920..12800}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {-1920} */

   int32_t temp_max_q6;
   /**< @h2xmle_description {Maximum reasonable measured temperature. A
                             temperature higher than this value is ignored.
                             (200 in degree C)}
        @h2xmle_range       {-1920..12800}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {12800} */

   int32_t temp_max_chg_sec_q6;
   /**< @h2xmle_description {Maximum temperature change per second. A new
                             temperature is ignored if it falls outside this
                             range.(Set 0 to disable this feature)}
        @h2xmle_range       {0..32767}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {4096} */

   int32_t temp_smoothing_factor_q31;
   /**< @h2xmle_description {Smoothing of the temperature to compute the gain.}
        @h2xmle_dataFormat  {Q31}
        @h2xmle_default     {6735944} */

   int32_t max_attenuation_db_q7;
   /**< @h2xmle_description {Minimum temperature control gain.}
        @h2xmle_range       {-7680..0}
        @h2xmle_dataFormat  {Q7}
        @h2xmle_default     {-1536} */

   int32_t attack_gain_smoothing_factor;
   /**< @h2xmle_description {Smoothing factor for adjusting gain during the
                             attack phase in q15 format (current gain less
                             than previous gain).}
        @h2xmle_dataFormat  {Q15}
        @h2xmle_range       {328..32767}
        @h2xmle_default     {32767} */

   int32_t release_gain_smoothing_factor;
   /**< @h2xmle_description {Smoothing factor for adjusting gain during the
                             release phase in q15 format (current gain greater
                             than previous gain).}
        @h2xmle_dataFormat  {Q15}
        @h2xmle_range       {328..32767}
        @h2xmle_default     {32767} */

   int32_t temp_dead_zone_q6;
   /**< @h2xmle_description {Dead zone (region near the target temperature) for a
                             reduced proportional term.(in degree C)}
        @h2xmle_range       {2..2048}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {64} */

   int32_t dead_zone_scaling_q15;
   /**< @h2xmle_description {Scaling factor to modify the proportional term.}
        @h2xmle_range       {328..32767}
        @h2xmle_dataFormat  {Q15}
        @h2xmle_default     {32767} */

   int32_t clip_time_out_ms;
   /**< @h2xmle_description {Timeout in milli seconds used for processing a clip
                             interrupt.(value greater than 0 enables the clip
                             interrupt timeout)}
        @h2xmle_range       {0..4000}
        @h2xmle_default     {0} */

   int32_t vbat_time_out_ms;
   /**< @h2xmle_description {Timeout in milli seconds used to process a VBAT
                             interrupt. (value greater than 0 enables the VBAT
                             interrupt timeout)}
        @h2xmle_range       {0..1000000}
        @h2xmle_default     {300000} */

   int32_t link_all_channels_gain;
   /**< @h2xmle_description {Apply common gain to both channels}
       @h2xmle_rangeList   {"Unlinked"=0; "Linked"=1}
        @h2xmle_default     {0} */

   int32_t max_release_time_ms;
   /**< @h2xmle_description {Maximum release time in msec}
        @h2xmle_range       {1000..4000}
        @h2xmle_default     {1000} */

   int32_t dyn_proc_type;
   /**< @h2xmle_description {Dynamic processing feature indicator}
        @h2xmle_rangeList   {"Disable"=0; "Notch filter"=1; "Bass boost"=2;
                      "Notch filter with resonance tracking"=3}
        @h2xmle_default     {0}
        @h2xmle_group       {Dyn_Notch} */

   int32_t dyn_proc_fc_q20;
   /**< @h2xmle_description {Dynamic processing -- Parametric EQ
                             filter frequency in Hz}
        @h2xmle_range       {0x06400000..0x7D000000} //range is 100 Hz to 2000 Hz
        @h2xmle_dataFormat  {Q20}
        @h2xmle_default     {0x0FA00000}
        @h2xmle_group       {Dyn_Notch} */

   int32_t dyn_proc_Q_q24;
   /**< @h2xmle_description {Dynamic processing -- Parametric EQ filter
                             queue}
        @h2xmle_range       {1677721..1677721600}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {50331648}
        @h2xmle_group       {Dyn_Notch} */

   int32_t dyn_proc_threshold_dBq23;
   /**< @h2xmle_description {Dynamic processing -- compression threshold in dB}
        @h2xmle_range       {-754974720..0}
        @h2xmle_dataFormat  {Q23}
        @h2xmle_default     {0}
        @h2xmle_group       {Dyn_Notch} */

   int32_t dyn_proc_ratio_q24;
   /**< @h2xmle_description {Dynamic processing -- compression ratio}
        @h2xmle_range       {16777216 .. 1677721600}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {16777216}
        @h2xmle_group       {Dyn_Notch} */

   int32_t dyn_proc_makeup_gain_q27;
   /**< @h2xmle_description {Dynamic processing -- makeup gain in linear scale
                             Default in decimal : 1
                             Range in decimal : 0.5 - 15.849}
        @h2xmle_range       {8468566..2127207634}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {134217728} */

   int32_t dyn_proc_attack_time_ms;
   /**< @h2xmle_description {dynamic processing -- attack time in msec}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {10}
        @h2xmle_group       {Dyn_Notch} */

   int32_t dyn_proc_release_time_ms;
   /**< @h2xmle_description {dynamic processing -- release time in msec}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {100}
        @h2xmle_group       {Dyn_Notch} */

   int32_t dyn_proc_auto_release_flag;
   /**< @h2xmle_description {flag for automatic release time in dynamic processing}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {1} */

   int32_t lfsn_type;
   /**< @h2xmle_description {optional excursion control using low frequency shelving or notch filter}
        @h2xmle_rangeList   {"Disable"=0; "Low frequency shelving"=1; "notch filter at resonance frequency"=2}
        @h2xmle_default     {0}
        @h2xmle_group       {Low_Freq_Shelf} */

   int32_t lfsn_fc_q20;
   /**< @h2xmle_description {frequency setting for low frequency shelving}
        @h2xmle_range       {104857600..1048576000}
        @h2xmle_dataFormat  {Q20}
        @h2xmle_default     {314572800}
        @h2xmle_group       {Low_Freq_Shelf} */

   int32_t lfsn_Q_q24;
   /**< @h2xmle_description {Q setting for time varying notch filter at resonance frequency}
        @h2xmle_range       {1677721..1677721600}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {50331648}
        @h2xmle_group       {Low_Freq_Shelf} */

   int32_t lfsn_min_gain_q27;
   /**< @h2xmle_description {minimum gain of low frequency shelving or notch filter
                             Default in decimal: 0.5
                             Range in decimal: 0 - 1}
        @h2xmle_range       {8468566..134217728}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {67268211}
        @h2xmle_group       {Low_Freq_Shelf} */

   int32_t lfsn_attack_time_ms;
   /**< @h2xmle_description {attack time in msec for time varying gain of shelving or notch filter}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {10} */

   int32_t lfsn_release_time_ms;
   /**< @h2xmle_description {release time in msec for time varying gain of shelving or notch filter}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {100} */

   int32_t lfsn_auto_release_flag;
   /**< @h2xmle_description {flag for automatic release time in low frequency shelving or notch filter}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {1} */

   int32_t exc_ctrl_attack_time_ms;
   /**< @h2xmle_description {attack time in msec for excursion limiter}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {0}
        @h2xmle_group       {Exc_Control} */

   int32_t exc_ctrl_release_time_ms;
   /**< @h2xmle_description {release time in msec for excursion limiter}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {100}
        @h2xmle_group       {Exc_Control} */

   int32_t exc_ctrl_auto_release_flag;
   /**< @h2xmle_description {flag for automatic release time in excursion limiter}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {1} */

   int32_t exc_ctrl_min_gain_q27;
   /**< @h2xmle_description {minimum gain for excursion limiter
                             Default in decimal: 0.25
                             Range in decimal: 0 - 1}
        @h2xmle_range       {8468566..134217728}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {33713968} */

   int32_t pow_lim_enable_flag;
   /**< @h2xmle_description {enablement flag for power limiter}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_default     {0} */

   int32_t limiter_attack_time_ms;
   /**< @h2xmle_description {amplitude and power limiter attack time in msec}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {0} */

   int32_t limiter_release_time_ms;
   /**< @h2xmle_description {amplitude and power limiter release time in msec}
        @h2xmle_range       {0..1000}
        @h2xmle_default     {100} */

   int32_t limiter_auto_release_flag;
   /**< @h2xmle_description {flag for automatic release time in limiter}
        @h2xmle_rangeList   {"disable"=0;"enable"=1}
        @h2xmle_default     {1} */

   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.
                             The parameters below depend on num channel and
                             are dynamically allocated based on num_ch value
                             specified.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   uint32_t pi_scale_u32q18[0];
/**< @h2xmle_description {scale down a full scale 40 Hz pilot
                          tone by this value.}
     @h2xmle_variableArraySize  {num_ch}
     @h2xmle_dataFormat  {Q18}
     @h2xmle_range       {0..65535}
     @h2xmle_default     {5230} */

#ifdef __H2XML__
   uint32_t rx_scale_u32q16[0];
   /**< @h2xmle_description {scale down the outgoing signal by this
                             value so when the pilot tone is added, the signal
                             does not exceed full scale.}
        @h2xmle_variableArraySize  {num_ch}
        @h2xmle_dataFormat  {Q16}
        @h2xmle_range       {32768..65535}
        @h2xmle_default     {64228} */

   int32_t r_spk_coil_q8[0];
   /**< @h2xmle_description {typical resistance. (in ohm)}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {512..16384}
        @h2xmle_dataFormat  {Q8}
        @h2xmle_default     {2048} */

   int32_t v_full_scale_q10[0];
   /**< @h2xmle_description {voltage for a digitally full scale
                             signal.}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {512..15360}
        @h2xmle_dataFormat  {Q10}
        @h2xmle_default     {5477} */

   int32_t thermal_resistance_q6[0];
   /**< @h2xmle_description {total thermal resistance (such as
                             voice coil + magnet).}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..32767}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {3200} */

   int32_t thermal_time_constant_low_q10[0];
   /**< @h2xmle_description {low estimate of the voice coil thermal
                             time constant. (in seconds)}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {51..32767}
        @h2xmle_dataFormat  {Q10}
        @h2xmle_default     {1024} */

   int32_t thermal_time_constant_high_q10[0];
   /**< @h2xmle_description {high estimate of voice coil thermal
                             time constant. (in seconds)}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {51..32767}
        @h2xmle_dataFormat  {Q10}
        @h2xmle_default     {3482} */

   int32_t temp_tgt_q6[0];
   /**< @h2xmle_description {target temperature.(in degrees C)}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {-1920..12800}
        @h2xmle_dataFormat  {Q6}
        @h2xmle_default     {5120} */

   int32_t pi_ctrl_prop_value_q12[0];
   /**< @h2xmle_description {value of the proportional term in the
                             PI controller.}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..32767}
        @h2xmle_dataFormat  {Q12}
        @h2xmle_default     {5734} */

   int32_t pi_ctrl_intg_value_q12[0];
   /**< @h2xmle_description {value of the integral term in the PI
                             controller.}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..32767}
        @h2xmle_dataFormat  {Q12}
        @h2xmle_default     {2294} */

   int32_t amp_gain_q24[0];
   /**< @h2xmle_description {speaker peak voltage for a digitally full-scale signal
                             Default in decimal: 8
                             Range in decimal: 0 - 128}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {134217728} */

   int32_t ptone_excursion_mm_q27[0];
   /**< @h2xmle_description {excursion for the pilot tone. This
                             excursion is added to the computed peak excursion.}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..134217728}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {0}
        @h2xmle_group       {Exc_Control} */

   int32_t dc_excursion_mm_q27[0];
   /**< @h2xmle_description {DC excursion margin}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..134217728}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {0}
        @h2xmle_group       {Exc_Control} */

   int32_t max_excursion_mm_q27[0];
   /**< @h2xmle_description {maximum permissible excursoin}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {40265318}
        @h2xmle_group       {Exc_Control} */

   int32_t max_amplitude_q27[0];
   /**< @h2xmle_description {amplitude limiter threshold in linear scale}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {134217728}
        @h2xmle_group       {Limiter} */

   int32_t max_power_W_q27[0];
   /**< @h2xmle_description {maximum power that amplifier can handle}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q27}
        @h2xmle_default     {536870912} */

   int32_t trace_resistance_rx_q24[0];
   /**< @h2xmle_description {trace resistance from amp output to speakers in Ohm}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {0} */

   int32_t Re_ohm_q24[0];
   /**< @h2xmle_description {DC resistance of voice coil at room temperature or small signal level in Ohm}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {33554432..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {134217728} */

   int32_t Bl_q24[0];
   /**< @h2xmle_description {Force factor (Bl product)}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {16777216} */

   int32_t Mms_gram_q24[0];
   /**< @h2xmle_description {Ch 0: Mechanical mass of loudspeaker diaphragm in gram}
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

   int32_t Qms_q24[0];
   /**< @h2xmle_description {Mechanical Q-factor}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q24}
        @h2xmle_default     {50331648} */

   int32_t Fres_Hz_q20[0];
   /**< @h2xmle_description {Resonance frequency in Hz}
        @h2xmle_variableArraySize  {num_ch}
       @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q20}
        @h2xmle_default     {838860800} */

   sp_tdf2_cfg_t band_splitting_even_filter_cfg[0];
   /**< @h2xmle_description {Band splitting even config}
        @h2xmle_variableArraySize  {num_ch} */

   sp_tdf2_cfg_t band_splitting_odd_filter_cfg[0];
/**< @h2xmle_description {Band splitting odd config}
     @h2xmle_variableArraySize  {num_ch} */

#endif //__H2XML__
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Structure definition for Parameter */
#define PARAM_ID_SP_TH_DEMO_CFG 0x080011ED

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_th_demo_cfg_t param_id_sp_th_demo_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_DEMO_CFG",
                         PARAM_ID_SP_TH_DEMO_CFG}
    @h2xmlp_description {parameter used for setting the algorithm internal
                         variables related to demo configuration of thermal processing.}
    @h2xmlp_toolPolicy  {CALIBRATION} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_demo_cfg_t
{
   uint16_t dsr;
   /**< @h2xmle_description {downsample ratio from internal values to output.}
        @h2xmle_default     {10}  */

   uint16_t lpass_alph_q16;
   /**< @h2xmle_description {lowpass smoothing factor. Q16 format.}
        @h2xmle_default     {512U}  */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Structure definition for Parameter */
#define PARAM_ID_SP_EX_DEMO_CFG 0x080011EE

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_ex_demo_cfg_t param_id_sp_ex_demo_cfg_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_DEMO_CFG",
                         PARAM_ID_SP_EX_DEMO_CFG}
    @h2xmlp_description {parameter used for setting the algorithm internal
                         variables related to demo configuration of excursion.}
    @h2xmlp_toolPolicy  {CALIBRATION} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_demo_cfg_t
{
   uint16_t dsr;
   /**< @h2xmle_description {downsample ratio from internal values to output.}
        @h2xmle_default     {10}  */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Structure definition for Parameter */
#define PARAM_ID_SP_TH_STATS 0x080011EF

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_th_stats_t param_id_sp_th_stats_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TH_STATS",
                         PARAM_ID_SP_TH_STATS}
    @h2xmlp_description {parameter used for RTM of the algorithm internal
                         variables related to thermal processing.}
    @h2xmlp_toolPolicy  {CALIBRATION} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_th_stats_t
{
   uint32_t frame_duration_in_ms;
   /**< @h2xmle_description {Frame duration in milliseconds for Thermal Rx
                             statistics.} */

   uint32_t frame_num;
   /**< @h2xmle_description {Frame number of the Thermal Rx statistics.} */

   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   int32_t vc_resis_q24[0][SP_TH_RX_DEMO_SMPL_PER_PKT];
/**< @h2xmle_description {Voice coil resistance array (in Ohms). }
     @h2xmle_dataFormat  {Q24}
     @h2xmle_range       {-2147483648..2147483647}   */

#ifdef __H2XML__
   int32_t vc_temp_q22[0][SP_TH_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Voice coil temperature array (in degrees C)}
        @h2xmle_dataFormat  {Q22} */

   int32_t th_gain_lnk_db_q23[0][SP_TH_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Gain control array (in deciBels)}
        @h2xmle_dataFormat  {Q23} */

   int32_t target_temp_q22[0];
/**< @h2xmle_description {Targeted temperature at which temperature
                          control kicks in.}
     @h2xmle_dataFormat  {Q22} */
#endif //__H2XML__
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_EX_STATS 0x080011F0

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_ex_stats_t param_id_sp_ex_stats_t;
/** @h2xmlp_parameter   {"PARAM_ID_SP_EX_STATS",
                         PARAM_ID_SP_EX_STATS}
    @h2xmlp_description {parameter used for RTM of the algorithm internal
                         variables related to exertion processing.}
    @h2xmlp_toolPolicy  {CALIBRATION} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_ex_stats_t
{
   uint32_t frame_duration_in_ms;
   /**< @h2xmle_description {Frame duration in milliseconds for Excursion Rx
                             statistics.} */

   uint32_t frame_num;
   /**< @h2xmle_description {Frame number of the Excursion Rx statistics.} */

   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   int32_t Re_rt_q24[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
/**< @h2xmle_description {DC resistance of voice coil at room
                          temperature or small signal level in Ohm.}
     @h2xmle_dataFormat  {Q24} */

#ifdef __H2XML__
   int32_t Bl_rt_q24[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Force factor (Bl product).}
        @h2xmle_dataFormat  {Q24} */

   int32_t Rms_rt_q24[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Mechanical damping or resistance of
                             loudspeaker in Kg/sec.}
        @h2xmle_dataFormat  {Q24} */

   int32_t Kms_rt_q24[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Mechanical stiffness of driver suspension in N/mm.}
        @h2xmle_dataFormat  {Q24} */

   int32_t Fres_rt_q20[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Resonance frequency in Hz.}
        @h2xmle_dataFormat  {Q20} */

   int32_t Qms_rt_q24[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Mechanical Q-factor.}
        @h2xmle_dataFormat  {Q24} */

   int32_t dp_in_rms_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {dp input rms.}
        @h2xmle_dataFormat  {Q27} */

   int32_t dp_out_rms_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {dp output rms.}
        @h2xmle_dataFormat  {Q27} */

   int32_t dp_notch_gain_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {dp notch gain.}
        @h2xmle_dataFormat  {Q27} */

   int32_t dp_tgt_rms_q27[0];
   /**< @h2xmle_description {dp target rms.}
        @h2xmle_dataFormat  {Q27} */

   int32_t lfsn_in_peak_pred_exc_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {lfsn input peak predicted excursion.}
        @h2xmle_dataFormat  {Q27} */

   int32_t lfsn_out_peak_pred_exc_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {lfsn output peak predicted excursion.}
        @h2xmle_dataFormat  {Q27} */

   int32_t lfsn_filter_gain_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {lfsn filter gain.}
        @h2xmle_dataFormat  {Q27} */

   int32_t exc_ctrl_in_peak_pred_exc_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Excursion peak input control prediction.}
        @h2xmle_dataFormat  {Q27} */

   int32_t exc_ctrl_out_peak_pred_exc_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Excursion peak output control prediction.}
        @h2xmle_dataFormat  {Q27} */

   int32_t exc_ctrl_tot_out_peak_pred_exc_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Total Excursion peak output control.}
        @h2xmle_dataFormat  {Q27} */

   int32_t exc_ctrl_gain_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Excursion control gain.}
        @h2xmle_dataFormat  {Q27} */

   int32_t exc_ctrl_tgt_excursion_q27[0];
   /**< @h2xmle_description {Target Excursion control.}
        @h2xmle_dataFormat  {Q27} */

   int32_t amp_lim_in_peak_pred_amp_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Peak Limitter input amplitude.}
        @h2xmle_dataFormat  {Q27} */

   int32_t amp_lim_out_peak_pred_amp_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Peak Limitter output amplitude.}
        @h2xmle_dataFormat  {Q27} */

   int32_t pow_lim_in_peak_pred_pow_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Peak Limitter input power.}
        @h2xmle_dataFormat  {Q27} */

   int32_t pow_lim_out_peak_pred_pow_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
   /**< @h2xmle_description {Peak Limitter output power.}
        @h2xmle_dataFormat  {Q27} */

   int32_t amp_lim_tgt_amplitude_q27[0];
   /**< @h2xmle_description {EX Limitter Target Amplitude.}
        @h2xmle_dataFormat  {Q27} */

   int32_t pow_lim_tgt_power_q27[0];
   /**< @h2xmle_description {EX Limitter Target Power.}
        @h2xmle_dataFormat  {Q27} */

   int32_t tot_lim_gain_q27[0][SP_EX_RX_DEMO_SMPL_PER_PKT];
/**< @h2xmle_description {Total  EX limitter gain.}
     @h2xmle_dataFormat  {Q27} */
#endif //__H2XML__
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_STATS 0x080011F1

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_stats_t param_id_sp_stats_t;
/** @h2xmlp_parameter   {"PARAM_ID_SP_STATS",
                         PARAM_ID_SP_STATS}
    @h2xmlp_description {parameter used for RTM of the algorithm internal
                         variables.}
    @h2xmlp_toolPolicy  {CALIBRATION} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_stats_t
{
   uint32_t frame_duration_in_ms;
   /**< @h2xmle_description {Frame duration in milliseconds for Rx statistics.}*/

   uint32_t frame_num;
   /**< @h2xmle_description {Frame number of the Rx statistics.} */

   uint32_t num_speakers;
   /**< @h2xmle_description {Number of speakers.} */

   uint32_t features;
   /**< @h2xmle_description {Features enabled in the speaker protection
                             algorithm.}

        @h2xmle_bitField    {0x00000001}
        @h2xmle_bitName     {"Notch high-pass filter"}
        @h2xmle_description {Notch high-pass filter}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_bitFieldEnd

        @h2xmle_bitField    {0x00000002}
        @h2xmle_bitName     {"Thermal protection"}
        @h2xmle_description {Thermal protection}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_bitFieldEnd

        @h2xmle_bitField    {0x00000004}
        @h2xmle_bitName     {"Feedback excursion control"}
        @h2xmle_description {Feedback excursion control}
        @h2xmle_rangeList   {"disabled"=0;"enabled"=1}
        @h2xmle_bitFieldEnd */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_TMAX_XMAX_LOGGING 0x080011F2

/*==============================================================================
   Type definitions
==============================================================================*/

/* TMAX XMAX Logging channel dependent params sub structure */
typedef struct sp_tmax_xmax_params_t sp_tmax_xmax_params_t;

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
/** @h2xmlp_subStruct */
struct sp_tmax_xmax_params_t
{
   int32_t max_excursion;
   /**< @h2xmle_description {maximum excursion since the lastest grasp of xmax in mm.}
        @h2xmle_range       {-2147483648..2147483647}
        @h2xmle_dataFormat  {Q27} */

   uint32_t count_exceeded_excursion;
   /**< @h2xmle_description {number of periods when the monitored excursion exceeds
                             to and stays at Xmax during logging_count_period.}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q0} */

   int32_t max_temperature;
   /**< @h2xmle_description {maximum temperature since the lastest grasp of tmax in C.}
        @h2xmle_range       {-2147483648..2147483647}
        @h2xmle_dataFormat  {Q22} */

   uint32_t count_exceeded_temperature;
   /**< @h2xmle_description {number of periods when the monitored temperature exceeds
                             to and stays at Tmax during logging_count_period.}
        @h2xmle_range       {0..2147483647}
        @h2xmle_dataFormat  {Q0} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
/* Structure definition for Parameter */
typedef struct param_id_sp_tmax_xmax_logging_t param_id_sp_tmax_xmax_logging_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_TMAX_XMAX_LOGGING",
                         PARAM_ID_SP_TMAX_XMAX_LOGGING}
    @h2xmlp_description {parameter used to log max temp and max excursion logging.}
    @h2xmlp_toolPolicy  {RTC} */
#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_tmax_xmax_logging_t
{
   uint32_t num_ch;
   /**< @h2xmle_description {Number of channels for Rx signal.}
        @h2xmle_rangeList   {"1"=1;"2"=2}
        @h2xmle_default     {1} */

   sp_tmax_xmax_params_t tmax_xmax_params[0];
   /**< @h2xmle_description {TMAX XMAX logging params sub structure.}
        @h2xmle_variableArraySize  {num_ch} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/*==============================================================================
   Constants
==============================================================================*/

/* Unique Paramter id */
#define PARAM_ID_SP_VERSION 0x080011F3

/*==============================================================================
   Type definitions
==============================================================================*/

/* Structure definition for Parameter */
typedef struct param_id_sp_version_t param_id_sp_version_t;

/** @h2xmlp_parameter   {"PARAM_ID_SP_VERSION",
                         PARAM_ID_SP_VERSION}
    @h2xmlp_description {parameter used to get the version of the SP Library.}
    @h2xmlp_toolPolicy  {RTC} */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct param_id_sp_version_t
{
   uint32_t lib_version_low;
   /**< @h2xmle_description {Lower 32 bits of the 64-bit
                             library version number.} */

   uint32_t lib_version_high;
   /**< @h2xmle_description {Higher 32 bits of the 64-bit
                             library version number.} */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;

/** @} <-- End of the Module --> */
#endif /* __SP_RX_H__ */
