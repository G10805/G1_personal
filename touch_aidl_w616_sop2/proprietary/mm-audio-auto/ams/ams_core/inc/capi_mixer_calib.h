/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef CAPI_MIXER_CALIB_H_
#define CAPI_MIXER_CALIB_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define CAPI_MODULE_ID_MIXER              0x123A8000
#define CAPI_PARAM_ID_MIXER_ENABLE        0x123A8001
#define CAPI_PARAM_ID_MIXER_CONFIG        0x123A8002

/** @h2xmlp_parameter   {"AUDIO_MIXER_ENABLE", CAPI_PARAM_ID_MIXER_ENABLE}
    @h2xmlp_description {Structure for enabling} */
typedef struct {
    uint32_t enable_flag;
    /**< @h2xmle_description {Specifies whether the module is enabled.} */
    /**< @h2xmle_rangeList   {"Disable"=0; "Enable"=1}  */
} capi_mixer_enable_t;

#define MIXER_QFACT 27
#define MIXER_INVAL_IDX 255

typedef struct {
    uint8_t input_port;
    /**< @h2xmle_description {input port} set to invalid index to set output to zeros*/
    uint8_t input_channel;
    /**< @h2xmle_description {input channel} */
    uint8_t output_port;
    /**< @h2xmle_description {output port} */
    uint8_t output_channel;
    /**< @h2xmle_description {output channel} */
    int32_t gain;
    /**< @h2xmle_dataFormat  {Q27} */
    /**< @h2xmle_description {gain coefficients as Q27 value} */
} capi_mixer_tap_t;

/** @h2xmlp_parameter   {"AUDIO_MIXER_CONFIG", CAPI_PARAM_ID_MIXER_CONFIG}
    @h2xmlp_description {mixer configuration, specify memory access during process calls and} */
typedef struct {
    uint32_t num_taps;
    /**< @h2xmle_description {total number tap definitions} */
    capi_mixer_tap_t taps[];
    /**< @h2xmle_variableArraySize {num_tap} */
    /**< @h2xmle_description {tap structure} */
} capi_mixer_config_t;

#ifdef __cplusplus
}
#endif

#endif /* CAPI_MIXER_CALIB_H_ */
