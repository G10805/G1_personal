#ifndef _CODEC_DMA_API_H_
#define _CODEC_DMA_API_H_
/**
 * \file codec_dma_api.h
 * \brief 
 *  	 This file contains CODEC_DMA module APIs
 * 
 * \copyright
 *  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/modules/codec_dma_api.h#18 $
*/
// clang-format on

#include "hw_intf_cmn_api.h"

/*==============================================================================
   Constants
==============================================================================*/
/* Input port ID of Codec DMA module */
#define PORT_ID_GEN_CNTR_DMA_INPUT   0x2

/* Output port ID of Codec DMA module */
#define PORT_ID_GEN_CNTR_DMA_OUTPUT  0x1

/* Stack size for Codec DMA Module*/
#define GEN_CNTR_DMA_STACK_SIZE          2048
/*==============================================================================
   Type definition
==============================================================================*/

/** Parameter used to configure the CODEC DMA interface  */

#define PARAM_ID_CODEC_DMA_INTF_CFG                            0x08001063

/** @h2xmlp_parameter   {"PARAM_ID_CODEC_DMA_INTF_CFG",
                         PARAM_ID_CODEC_DMA_INTF_CFG}
    @h2xmlp_description {Configures the CODEC DMA interface.}
    @h2xmlp_toolPolicy   {Calibration} */

#include "spf_begin_pack.h"
/** Payload for parameter PARAM_ID_CODEC_DMA_INTF_CFG */
struct param_id_codec_dma_intf_cfg_t
{
       uint32_t                lpaif_type;
       /**< @h2xmle_description {LPAIF types.}
            @h2xmle_rangeList   {"LPAIF"=0;
                                 "LPAIF_RXTX"=1;
                                 "LPAIF_WSA"=2;
                                 "LPAIF_VA"=3;
                                 "LPAIF_AXI"=4}
          @h2xmle_default     {2}
     */
     uint32_t                  intf_indx;
     /**< @h2xmle_description {CODEC DMA interface index which picks an instance of codec dma. 
	                           Each interface supports different num of channels}
          @h2xmle_rangeList   {"CODEC_RX0"=1;
                               "CODEC_TX0"=1;
                               "CODEC_RX1"=2;
                               "CODEC_TX1"=2;
                               "CODEC_RX2"=3;
                               "CODEC_TX2"=3;
                               "CODEC_RX3"=4;
                               "CODEC_TX3"=4;
                               "CODEC_RX4"=5;
                               "CODEC_TX4"=5;
                               "CODEC_RX5"=6;
                               "CODEC_TX5"=6;
                               "CODEC_RX6"=7;
                               "CODEC_RX7"=8}
          @h2xmle_default     {1}
     */
     uint32_t                  active_channels_mask;

     /**< @h2xmle_description {Active channels mask to denote the bit mask for active channels.
          Bits 0 to 7 denote channels 0 to 7. A set bit in the mask denotes
          the channel is active while a 0 denotes a channel is inactive. 
		  Mask should match the channel indices being set from codec side}
          @h2xmle_range       {1..255}
          @h2xmle_default     {1}
     */
}
#include "spf_end_pack.h"
;

/* Structure type def for above payload. */
typedef struct param_id_codec_dma_intf_cfg_t param_id_codec_dma_intf_cfg_t;

/*------------------------------------------------------------------------------
   Module
------------------------------------------------------------------------------*/
 /** @h2xml_title1           {Codec Dma Module API}
     @h2xml_title_agile_rev  {Codec Dma Module API}
     @h2xml_title_date       {Oct 11, 2018} */
     
/** Module ID for CODEC_DMA sink module */
#define MODULE_ID_CODEC_DMA_SINK                                      0x07001023
/** @h2xmlm_module       {"MODULE_ID_CODEC_DMA_SINK",
                           MODULE_ID_CODEC_DMA_SINK}
    @h2xmlm_displayName  {"Codec DMA Sink"}
    @h2xmlm_description  {CODEC_DMA Sink Module\n
                        - Supports following params:
                        - PARAM_ID_CODEC_DMA_INTF_CFG \n
                          - PARAM_ID_HW_EP_MF_CFG \n
                          - PARAM_ID_HW_EP_FRAME_SIZE_FACTOR \n
                          - PARAM_ID_HW_EP_POWER_MODE_CFG\n
                          - PARAM_ID_HW_EP_DMA_DATA_ALIGN\n
                          - \n
                          - Supported Input Media Format: \n
                          - Data Format          : FIXED_POINT \n
                          - fmt_id               : Don't care \n
                          - Sample Rates         : 8, 11.025, 12, 16, 22.05, 24, 32, 44.1, 48, \n
                          -                        88.2, 96, 176.4, 192, 352.8, 384 kHz \n
                          - Number of channels   : 1 to 8 \n
                          - Channel type         : Don't care \n
                          - Bit Width            : 16 (bits per sample 16 and Q15), \n
                          -                      : 24 (bits per sample 24 and Q27) \n
                          -                      : 32 (bits per sample 32 and Q31)  \n
                          - Q format             : Q15, Q27, Q31 \n
                          - Interleaving         : I2S sink module needs de-interleaved unpacked }
    @h2xmlm_dataInputPorts      {IN = PORT_ID_GEN_CNTR_DMA_INPUT}
    @h2xmlm_dataMaxInputPorts   {1}
    @h2xmlm_dataMaxOutputPorts  {0}
    @h2xmlm_ctrlDynamicPortIntent  { "Qtimer HW-EP drift info"=INTENT_ID_TIMER_DRIFT_INFO, maxPorts= -1 }
    @h2xmlm_ctrlDynamicPortIntent  { "HW EP Module Instance info"=INTENT_ID_MODULE_INSTANCE_INFO, maxPorts= -1 }
    @h2xmlm_supportedContTypes { APM_CONTAINER_TYPE_GC }
    @h2xmlm_isOffloadable       {false}
    @h2xmlm_stackSize           { GEN_CNTR_DMA_STACK_SIZE }
    @{                   <-- Start of the Module -->

    @h2xml_Select     {param_id_hw_ep_mf_t}
    @h2xmlm_InsertParameter
    @h2xml_Select        {param_id_hw_ep_mf_t::num_channels}
    @h2xmle_rangeList    {"ONE"=1;
                         "TWO"=2;
                         "THREE"=3;
                         "FOUR"=4;
                         "FIVE"=5;
                         "SIX"=6;
                         "SEVEN"=7;
                         "EIGHT"=8}
    @h2xmle_default      {1}
    @h2xml_Select        {param_id_hw_ep_mf_t::data_format}
    @h2xmle_rangeList    {"DATA_FORMAT_FIXED_POINT"=1}
    @h2xmle_default      {1}

    @h2xml_Select     {param_id_frame_size_factor_t}
    @h2xmlm_InsertParameter
    @h2xml_Select      {param_id_hw_ep_power_mode_cfg_t}
    @h2xmlm_InsertParameter
    @h2xml_Select     {param_id_codec_dma_intf_cfg_t}
    @h2xmlm_InsertParameter
    @h2xml_Select        {param_id_codec_dma_intf_cfg_t::intf_indx}
    @h2xmle_rangeList    {"CODEC_RX0"=1;
                         "CODEC_RX1"=2;
                         "CODEC_RX2"=3;
                         "CODEC_RX3"=4;
                         "CODEC_RX4"=5;
                         "CODEC_RX5"=6;
                         "CODEC_RX6"=7;
                         "CODEC_RX7"=8}
    @h2xmle_default      {1}

    @h2xml_Select     {param_id_hw_ep_dma_data_align_t}
    @h2xmlm_InsertParameter
    @h2xml_Select        {param_id_hw_ep_dma_data_align_t::dma_data_align}
    @h2xmle_rangeList    {"DMA_DATA_ALIGN_MSB"=0;
                         "DMA_DATA_ALIGN_LSB"=1}
    @h2xmle_default      {0}
    @}                   <-- End of the Module -->*/

/** Module ID for CODEC_DMA source module  */
#define MODULE_ID_CODEC_DMA_SOURCE                                  0x07001024
/** @h2xmlm_module       {"MODULE_ID_CODEC_DMA_SOURCE",
                           MODULE_ID_CODEC_DMA_SOURCE}
    @h2xmlm_displayName  {"Codec DMA Source"}
    @h2xmlm_description  {CODEC_DMA Source Module \n
                          - Supports following params:
                        - PARAM_ID_CODEC_DMA_INTF_CFG \n
                          - PARAM_ID_HW_EP_MF_CFG \n
                          - PARAM_ID_HW_EP_FRAME_SIZE_FACTOR\n
                          - PARAM_ID_HW_EP_POWER_MODE_CFG\n
                          - PARAM_ID_HW_EP_SRC_CHANNEL_MAP\n
                          - \n
                          - Supported Input Media Format: \n
                          - Codec dma source module doesn't take any inputs }
    @h2xmlm_dataOutputPorts     {OUT = PORT_ID_GEN_CNTR_DMA_OUTPUT}
    @h2xmlm_dataMaxInputPorts   {0}
    @h2xmlm_dataMaxOutputPorts  {1}
    @h2xmlm_ctrlDynamicPortIntent  { "Qtimer HW-EP drift info"=INTENT_ID_TIMER_DRIFT_INFO, maxPorts= -1 }
    @h2xmlm_ctrlDynamicPortIntent  { "HW EP Module Instance info"=INTENT_ID_MODULE_INSTANCE_INFO, maxPorts= -1 }
    @h2xmlm_supportedContTypes { APM_CONTAINER_TYPE_GC }
    @h2xmlm_stackSize           { GEN_CNTR_DMA_STACK_SIZE }
    @{                   <-- Start of the Module -->

    @h2xml_Select     {param_id_hw_ep_mf_t}
    @h2xmlm_InsertParameter
    @h2xml_Select        {param_id_hw_ep_mf_t::num_channels}
    @h2xmle_rangeList    {"ONE"=1;
                         "TWO"=2;
                         "THREE"=3;
                         "FOUR"=4;
                         "FIVE"=5;
                         "SIX"=6;
                         "SEVEN"=7;
                         "EIGHT"=8}
    @h2xmle_default      {1}
	@h2xml_Select        {param_id_hw_ep_mf_t::data_format}
	@h2xmle_rangeList    {"DATA_FORMAT_FIXED_POINT"=1}
	@h2xmle_default      {1}

    @h2xml_Select    {param_id_frame_size_factor_t}
    @h2xmlm_InsertParameter
    @h2xml_Select      {param_id_hw_ep_power_mode_cfg_t}
    @h2xmlm_InsertParameter
    @h2xml_Select        {param_id_codec_dma_intf_cfg_t}
    @h2xmlm_InsertParameter
    @h2xml_Select        {param_id_codec_dma_intf_cfg_t::intf_indx}
    @h2xmle_rangeList    {"CODEC_TX0"=1;
                         "CODEC_TX1"=2;
                         "CODEC_TX2"=3;
                         "CODEC_TX3"=4;
                         "CODEC_TX4"=5;
                         "CODEC_TX5"=6}
    @h2xmle_default      {1}

    @h2xml_Select        {param_id_hw_ep_src_channel_map_t}
    @h2xmlm_InsertParameter

    @}                   <-- End of the Module -->*/
#endif //_CODEC_DMA_API_H_
