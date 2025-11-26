#ifndef _DSP_AMS_DELAY_H_
#define _DSP_AMS_DELAY_H_

/*===========================================================================
Copyright (c) 2019,2022-2023 Qualcomm Technologies, Inc.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================ */

/**
  ID of the delay module.

  This module introduces the specified amount of delay in the audio path.
  If the delay is increased, silence is inserted. If the delay is decreased,
  data is dropped.

  There are no smooth transitions. The resolution of the delay applied is
  limited by the period of a single sample.

  This module supports the following parameter IDs.
  - DSP_AMS_PARAM_ID_DELAY
  - DSP_AMS_PARAM_ID_MCHAN_DELAY
*/
#define DSP_AMS_MODULE_ID_DELAY (0x00013375)

/** ID of the delay parameter used by DSP_AMS_MODULE_ID_DELAY. */
#define DSP_AMS_PARAM_ID_DELAY (0x00013376)

/** Payload structure for DSP_AMS_PARAM_ID_DELAY. */
typedef struct dsp_ams_delay_param_t dsp_ams_delay_param_t;

struct dsp_ams_delay_param_t
{
     uint32_t delay_us;
     /**< Delay in microseconds. Supported values: 0 - 100000
          The amount of delay must be greater than 0. If the value is zero, this
          module is disabled. The actual resolution of the delay is limited by
          the period of a single audio sample. */
};

/** ID of the multi-channel delay parameter used by DSP_AMS_MODULE_ID_DELAY. */
#define DSP_AMS_PARAM_ID_MCHAN_DELAY (0x00013377)

/** Structure for per channel delay. */
typedef struct dsp_ams_param_channel_delay_config_t dsp_ams_param_channel_delay_config_t;

struct dsp_ams_param_channel_delay_config_t
{
     uint32_t channel_mask_lsb;
     /**< Lower 32 bits of the mask that indicates the corresponding channel
          whose delay is to be set.
          - Set the bits corresponding to 1 to 31 channels of standard channel
            mapping (channels are mapped per standard channel mapping)
          - Position of the bit to set 1 (left shift) (channel_map) */

     uint32_t channel_mask_msb;
     /**< Upper 32 bits of the mask that indicates the corresponding channel
          whose delay is to be set.
          - Set the bits corresponding to 32 to 63 channels of standard channel
            mapping (channels are mapped per standard channel mapping)
          - Position of the bit to set  1 (left shift) (channel_map - 32) */

     uint32_t delay_us;
     /**< Delay in microseconds.
          The amount of delay must be greater than 0. If the value is zero,
          this module is disabled.
          The actual resolution of the delay is limited by the period of a
          single audio sample. */
};

/** Payload structure for AMS_PARAM_ID_MCHAN_DELAY. */
typedef struct dsp_ams_mchan_delay_param_t dsp_ams_mchan_delay_param_t;

struct dsp_ams_mchan_delay_param_t
{
     uint32_t num_config;
     /**< Number of channel delay configurations.
          Supported values: 1 - 63 */

     /** Followed by array below.
         The start address for the array must be 4-byte aligned.
         dsp_ams_param_channel_delay_config_t delays[num_config]; */
};

#endif /* _DSP_AMS_DELAY_H_ */
