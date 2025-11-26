#ifndef _AMS_TST_CASE1_ALGO_H_
#define _AMS_TST_CASE1_ALGO_H_

/*===========================================================================
Copyright (c) 2019 Qualcomm Technologies, Inc.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================ */

/**
  ID of the TST_CASE1_ALGO module.
*/
#define DSP_AMS_TST_CASE1_ALGO_MODULE_ID ( 0x123F0000 )

/** Specify module valid input port idx
  * and/or port channle infor
  */
#define DSP_AMS_TST_CASE1_ALGO_PARAM_ID_INPUT_INFOR ( 0x123F0001 )


/** Payload for the DSP_AMS_TST_CASE1_ALGO_PARAM_ID_INPUT_INFO parameter. */
typedef struct dsp_ams_tst_case1_input_param_t dsp_ams_tst_case1_input_param_t;

struct dsp_ams_tst_case1_input_param_t
{
    uint32_t which_ports;

    /** specify port_index */
};

/** query input port time_stamp
  *
  */
#define DSP_AMS_TST_CASE1_ALGO_PARAM_ID_TIMESTAMP ( 0x123F0002 )


/** Payload for the DSP_AMS_TST_CASE1_ALGO_PARAM_ID_TIMESTAMP parameter. */
typedef struct dsp_ams_tst_case1_timestamp_param_t dsp_ams_tst_case1_timestamp_param_t;

struct dsp_ams_tst_case1_timestamp_param_t
{
    uint64_t ts_port_0;
    uint64_t ts_port_1;
};


/** query input port time_stamp
  *
  */
#define DSP_AMS_TST_CASE1_ALGO_PARAM_ID_TIMESTAMP_2 ( 0x123F0003 )

#define DSP_AMS_TST_CASE1_ALGO_MAX_NUM_TS_PAIR       10
#define DSP_AMS_TST_CASE1_ALGO_MAX_NUM_TS_PORT       2


/** Payload for the DSP_AMS_TST_CASE1_ALGO_PARAM_ID_TIMESTAMP_2 parameter. */
typedef struct dsp_ams_tst_case1_timestamp_2_param_t dsp_ams_tst_case1_timestamp_2_param_t;

struct dsp_ams_tst_case1_timestamp_2_param_t
{
    uint64_t port0_ts[DSP_AMS_TST_CASE1_ALGO_MAX_NUM_TS_PAIR][DSP_AMS_TST_CASE1_ALGO_MAX_NUM_TS_PORT];  //port0_ts[i][0]  --- input_port_0, port0_ts[i][1]  --- output_port_0
    uint64_t port1_ts[DSP_AMS_TST_CASE1_ALGO_MAX_NUM_TS_PAIR][DSP_AMS_TST_CASE1_ALGO_MAX_NUM_TS_PORT];  //port1_ts[i][0]  --- input_port_1, port1_ts[i][1]  --- output_port_1
};


#endif /* _AMS_TST_CASE1_ALGO_H_ */

