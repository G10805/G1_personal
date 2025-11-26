#ifndef _DSP_AMS_PASSTHROUGH_H_
#define _DSP_AMS_PASSTHROUGH_H_

/*===========================================================================
Copyright (c) 2019,2022-2023 Qualcomm Technologies, Inc.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================ */

/**
  ID of the passthrough module.

  This module copies data from its input ports to its output ports.

  The copy operation is done based on the channel mapping configured on the
  input ports and output ports, i.e. the content of input channel with channel
  type X is copied to the corresponding output channel with channel type X.

  The channel mapping for each of the input ports is configured by the AMS
  framework; where the channel mapping is derived from the upstream element
  (module or endpoint) connected to the passthrough module.

  The channel mapping for each of the output ports is determined as follows:
  1. If the output port is connected to an endpoint, the channel mapping will
     be configured by the AMS framework where the mapping is derived from the
     endpoint.
  2. Otherwise,
     a. Client can configure the channel mapping using the
        DSP_AMS_PARAM_ID_OUTPUT_CHANNEL_MAP.
     b. If client doesn't configure the channel mapping, the passthrough module
        will configure the channel mapping of output port N to be the same as
        input port N. If the number of input port is less than N, the output
        channel mapping will not be configured.

  The module will not run if any of the output port doesn't have valid channel
  mapping.

  The passthrough module supports only CAPI V2 standard data format as defined
  by capi_v2_standard_data_format_t. The following data format configuration
  must be the same for all intput and output ports:
  - bitstream_format
  - bits_per_sample
  - q_factor
  - sampling_rate
  - data_is_signed
  - data_interleaving

  For data_interleaving, only CAPI_V2_DEINTERLEAVED_UNPACKED is supported.
*/
#define DSP_AMS_MODULE_ID_PASSTHROUGH (0x00013378)

/** Output channel mapping parameter used by DSP_AMS_MODULE_ID_PASSTHROUGH.
 * In the case where there is a failure for the DSP_AMS_CMD_SET_PARAM
 * command, the channel map will not be set on any of the output ports
 * specified in this command.
 */
#define DSP_AMS_PARAM_ID_OUTPUT_CHANNEL_MAP (0x00013379)

/** Per output port channel map. */
typedef struct dsp_ams_channel_map_t dsp_ams_channel_map_t;

struct dsp_ams_channel_map_t
{
   uint32_t port_index;
   /**< Index of the output port. */
   uint32_t num_channels;
   /**< Number of channels. */
   uint16_t channel_type[32];
   /**< Channel type as defined by 80-NF775-1. */
};

/** Payload for the DSP_AMS_PARAM_ID_OUTPUT_CHANNEL_MAP parameter. */
typedef struct dsp_ams_output_channel_map_param_t dsp_ams_output_channel_map_param_t;

struct dsp_ams_output_channel_map_param_t
{
   uint32_t num_ports;

   /** Followed by array below.
       The start address for the array must be 4-byte aligned.
       dsp_ams_channel_map_t channel_map[ num_ports ]; */
};

#endif /* _DSP_AMS_PASSTHROUGH_H_ */
