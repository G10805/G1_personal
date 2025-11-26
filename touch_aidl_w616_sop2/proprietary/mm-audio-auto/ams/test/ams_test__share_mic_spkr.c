/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#include <log/log.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "ams.h"
#include "ams_util.h"

#include "dsp_audio_micro_service.h"
#include "ams_tst_case1_algo.h"
#include "dsp_ams_passthrough.h"
#include "dsp_ams_delay.h"
#include "capi_mixer_calib.h"

#define AMS_TEST_MSG_PRINT_logger_log(level, format, ...) \
    do                                                    \
    {                                                     \
        switch (level)                                    \
        {                                                 \
        case AmsTestMsgLevel_Error:                       \
            ALOGE(format, ##__VA_ARGS__);                 \
            break;                                        \
        default:                                          \
            ALOGD(format, ##__VA_ARGS__);                 \
            break;                                        \
        }                                                 \
    } while (0)

#define __FILE_SHORT__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define AMS_TEST_MSG_PRINT(level, format, ...) \
    AMS_TEST_MSG_PRINT_logger_log(level, format, ##__VA_ARGS__)

#define GET_NXT_UNIQUE_ID__(x) (++x)


static ams_session_t ams_session;
static ams_graph_basic_params_t g_param;
static ams_graph_handle_t g_handle1 = NULL;
static ams_module_t m1;
static ams_module_t m2;

static int ams_set_input_port_share_mic_spkr(int32_t which_ports)
{
    int rc;
    ams_graph_handle_t *pgh = g_handle1;
    dsp_ams_tst_case1_input_param_t cfg;
    cfg.which_ports = which_ports;

    uint32_t param_size = sizeof(dsp_ams_tst_case1_input_param_t);

    rc = ams_set_param(ams_session, pgh, m2.id,
            DSP_AMS_TST_CASE1_ALGO_PARAM_ID_INPUT_INFOR, param_size, &cfg);

    printf("ams_set_input_port_share_mic_spkr: which_port %d, returned %d\n", which_ports, rc);

    return rc;
}


static int ams_get_timestamp_share_mic_spkr(void)
{
    int rc;
    ams_graph_handle_t *pgh = g_handle1;
    dsp_ams_tst_case1_timestamp_param_t cur_ts = {0};
    uint32_t pid = DSP_AMS_TST_CASE1_ALGO_PARAM_ID_TIMESTAMP;
    uint32_t param_size = sizeof(dsp_ams_tst_case1_timestamp_param_t);

    uint32_t port0_ts, port1_ts, delta_ts;
    rc = ams_get_param(ams_session,
                       pgh,
                       m2.id,
                       pid,
                       &param_size,
                       &cur_ts);

    port0_ts = (uint32_t)cur_ts.ts_port_0;
    port1_ts = (uint32_t)cur_ts.ts_port_1;

    printf("ams_get_timestamp_share_mic_spkr: cur_ts port_0=0x%08X port_1=0x%08X rc=%d\n",
            port0_ts, port1_ts, rc);

    if(port0_ts >= port1_ts)
    {
      delta_ts = port0_ts - port1_ts;
      printf("EXCLUSIVE_MIC_TS >= SHARE_MIC_TS: timestamp delta=0x%08X\n",
            delta_ts);
    }
    else
    {
      delta_ts = port1_ts - port0_ts;
      printf("EXCLUSIVE_MIC_TS < SHARE_MIC_TS: timestamp delta=0x%08X\n",
            delta_ts);
    }

    return rc;
}

static int my_TDM5_CFG_FLAG = 0;  //0   for 6155
                                  //1   for 8155/9155

//SHARE_MIC + SHARE_SPKR test case for 6155
static int setup_usecase_share_mic_spkr_6155(ams_session_t ams_session, ams_graph_basic_params_t* param, ams_graph_handle_t *gh)
{

// e2 [SHARE_TDM3_RX, 8 ch, 32 bit] -------------------------------------------> port0|---------|port0-----> e4 {EXCLUSIVE_TDM5_RX, 16 ch, 32 bit}
//                                                                                    |    m1   |
//                                                                                    |         |
// e5 [SHARE_TDM3_TX, 8 ch, 32 bit] ----------->port1|---------|port0----------->port1|         |
//                                                   |    m2   |                      |---------|port1-----> e1 [EXCLUSIVE_TDM2_RX, 8 ch, 32 bit]
// e3 [EXCLUSIVE_TDM2_TX, 8 ch, 32 bit]-------->port0|         |
//                                                   |---------|
    ams_status_t rc = 0;
    ams_endpoint_t e1, e2, e3, e4, e5;
    //ams_module_t m1, m2;
    ams_connection_t c1, c2, c3, c4, c5, c6;
    uint32_t id=1000;

    rc = ams_create_graph (ams_session, param, gh);

    // TDM2 RX - Mercury Speaker
    e1.id = GET_NXT_UNIQUE_ID__(id);
    e1.type = AMS_ENDPOINT_TYPE_SINK;
    e1.channel_mask = 0xFF;
    e1.q_factor = 31;
    e1.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset( e1.channel_type, 0, sizeof( e1.channel_type ) );
    e1.channel_type[0] = 0;
    e1.channel_type[1] = 1;
    e1.channel_type[2] = 2;
    e1.channel_type[3] = 3;
    e1.channel_type[4] = 4;
    e1.channel_type[5] = 5;
    e1.channel_type[6] = 6;
    e1.channel_type[7] = 7;

    e1.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM2;
    e1.tdm_params.num_channels = 8;
    e1.tdm_params.bit_width = 32;
    e1.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e1.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e1.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e1.tdm_params.nslots_per_frame = 8;
    e1.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e1.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e1.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e1.tdm_params.slot_width = 32;
    e1.tdm_params.slot_mask = 0xFF;

    // TDM3 RX - shared spkr endpoint
    e2.id = GET_NXT_UNIQUE_ID__(id);
    e2.type = AMS_ENDPOINT_TYPE_SOURCE;
    e2.channel_mask = 0xFF;  //0xFFFF;
    e2.q_factor = 31;
    e2.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT;

    memset( e2.channel_type, 0, sizeof( e2.channel_type ) );
    e2.channel_type[0] = 0;
    e2.channel_type[1] = 1;
    e2.channel_type[2] = 2;
    e2.channel_type[3] = 3;
    e2.channel_type[4] = 4;
    e2.channel_type[5] = 5;
    e2.channel_type[6] = 6;
    e2.channel_type[7] = 7;

    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
    e2.tdm_params.num_channels = 8;   //16;
    e2.tdm_params.bit_width = 32;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = 8;   //16;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = 0xFF;  //0xFFFF;

    // TDM2 TX - Mercury Mic [ANC MIC]
    e3.id = GET_NXT_UNIQUE_ID__(id);
    e3.type = AMS_ENDPOINT_TYPE_SOURCE;
    e3.channel_mask = 0xFF;
    e3.q_factor = 31;
    e3.flags = AMS_ENDPOINT_EXCLUSIVE;

    memset( e3.channel_type, 0, sizeof( e3.channel_type ) );
    e3.channel_type[0] = 0;
    e3.channel_type[1] = 1;
    e3.channel_type[2] = 2;
    e3.channel_type[3] = 3;
    e3.channel_type[4] = 4;
    e3.channel_type[5] = 5;
    e3.channel_type[6] = 6;
    e3.channel_type[7] = 7;

    e3.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM2;
    e3.tdm_params.num_channels = 8;
    e3.tdm_params.bit_width = 32;
    e3.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e3.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e3.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e3.tdm_params.nslots_per_frame = 8;
    e3.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e3.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e3.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e3.tdm_params.slot_width = 32;
    e3.tdm_params.slot_mask = 0xFF;

    // TDM5 RX - A2B Speaker
    e4.id = GET_NXT_UNIQUE_ID__(id);
    e4.type = AMS_ENDPOINT_TYPE_SINK;
    e4.channel_mask = 0xFFFF;
    e4.q_factor = 31;
    e4.flags = AMS_ENDPOINT_EXCLUSIVE;

    memset( e4.channel_type, 0, sizeof( e4.channel_type ) );
    e4.channel_type[0] = 0;
    e4.channel_type[1] = 1;
    e4.channel_type[2] = 2;
    e4.channel_type[3] = 3;
    e4.channel_type[4] = 4;
    e4.channel_type[5] = 5;
    e4.channel_type[6] = 6;
    e4.channel_type[7] = 7;

    e4.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM5;
    e4.tdm_params.num_channels = 16;
    e4.tdm_params.bit_width = 32;
    e4.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e4.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;

    if(my_TDM5_CFG_FLAG)
    {
        e4.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    }
    else
    {
        e4.tdm_params.sync_src = AMS_TDM_SYNC_SRC_EXTERNAL;  //AMS_TDM_SYNC_SRC_INTERNAL;
    }
    e4.tdm_params.nslots_per_frame = 16;
    e4.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e4.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e4.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e4.tdm_params.slot_width = 32;
    e4.tdm_params.slot_mask = 0xFFFF;

    ////
    // TDM3 TX - shared MIC endpoint
    e5.id = GET_NXT_UNIQUE_ID__(id);
    e5.type = AMS_ENDPOINT_TYPE_SOURCE;
    e5.channel_mask = 0xFF;
    e5.q_factor = 31;
    e5.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_INPUT;

    memset( e5.channel_type, 0, sizeof( e5.channel_type ) );
    e5.channel_type[0] = 0;
    e5.channel_type[1] = 1;
    e5.channel_type[2] = 2;
    e5.channel_type[3] = 3;
    e5.channel_type[4] = 4;
    e5.channel_type[5] = 5;
    e5.channel_type[6] = 6;
    e5.channel_type[7] = 7;

    e5.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
    e5.tdm_params.num_channels = 8;
    e5.tdm_params.bit_width = 32;
    e5.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e5.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e5.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e5.tdm_params.nslots_per_frame = 8;
    e5.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e5.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e5.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e5.tdm_params.slot_width = 32;
    e5.tdm_params.slot_mask = 0xFF;
    ////

    m1.id = GET_NXT_UNIQUE_ID__(id);
    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = 0x123E0000;   //CAPI_V2_ANC_MODULE_ID
    strlcpy (m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy (m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

    m2.id = GET_NXT_UNIQUE_ID__(id);
    m2.flags = AMS_INLINE_PROCESSING_MODE;
    m2.capiv2_info.id = 0x123F0000;    //CAPI_V2_DELAY_MODULE_ID
    strlcpy (m2.capiv2_info.tag, "", sizeof(m2.capiv2_info.tag));
    strlcpy (m2.capiv2_info.shared_obj_filename, "", sizeof(m2.capiv2_info.shared_obj_filename));

    // from shared endpoint TDM3 RX to M1 module
    c1.num_channels = 8;  //16;
    c1.bit_width = 32;
    c1.source.type =  AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.source.endpoint.id = e2.id;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.destination.module.id = m1.id;
    c1.destination.module.port_index = 0;

    // from source endpoint SEC TDM TX to M2 module, input_port_0
    c2.num_channels = 8;
    c2.bit_width = 32;
    c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c2.source.endpoint.id = e3.id;
    c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c2.destination.module.id = m2.id;
    c2.destination.module.port_index = 0;

    //from m1 to QUIN_TDM_RX
    c3.num_channels = 16;
    c3.bit_width = 32;
    c3.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c3.source.module.id = m1.id;
    //port index should be what?
    c3.source.module.port_index = 0;
    c3.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c3.destination.endpoint.id = e4.id;

    //FROM M2 TO M1
    c4.num_channels = 8;
    c4.bit_width = 32;
    c4.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c4.source.module.id = m2.id;
    c4.source.module.port_index = 0;
    c4.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c4.destination.module.id = m1.id;
    c4.destination.module.port_index = 1;

    //from m1 to SEC_TDM_RX
    c5.num_channels = 8;
    c5.bit_width = 32;
    c5.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c5.source.module.id = m1.id;
    c5.source.module.port_index = 1;
    c5.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c5.destination.endpoint.id = e1.id;

    ///
    // from SHARE_MIC TERT TDM TX to M2 module, input_port_1
    c6.num_channels = 8;
    c6.bit_width = 32;
    c6.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c6.source.endpoint.id = e5.id;
    c6.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c6.destination.module.id = m2.id;
    c6.destination.module.port_index = 1;
    ///

    rc = ams_graph_add_endpont (ams_session, *gh, &e4);

    rc = ams_graph_add_endpont (ams_session, *gh, &e1);

    rc = ams_graph_add_endpont (ams_session, *gh, &e2);

    rc = ams_graph_add_endpont (ams_session, *gh, &e3);

    rc = ams_graph_add_endpont (ams_session, *gh, &e5);

    rc = ams_graph_add_module (ams_session, *gh, &m1);

    rc = ams_graph_add_module (ams_session, *gh, &m2);

    rc = ams_graph_add_connection (ams_session, *gh, &c1);

    rc = ams_graph_add_connection (ams_session, *gh, &c2);

    rc = ams_graph_add_connection (ams_session, *gh, &c3);

    rc = ams_graph_add_connection (ams_session, *gh, &c4);

    rc = ams_graph_add_connection (ams_session, *gh, &c5);

    rc = ams_graph_add_connection (ams_session, *gh, &c6);

    rc = ams_util_print_graph_info (ams_session, *gh);

    rc = ams_open_graph (ams_session, *gh);

    if(*gh == NULL)
    {
        printf("ams_service: Unable to open graph, exiting ams service");
        return rc;
    }

    rc = ams_start_graph (ams_session, *gh);

    return rc;
}

//TIMESTAMP MSG LINK test case for 8155/8195/6150
static int usecase_msg_link_get_timestamp(void)
{
    int rc;
    ams_graph_handle_t *pgh = g_handle1;
    dsp_ams_tst_case1_timestamp_2_param_t cur_ts;
    uint32_t pid = DSP_AMS_TST_CASE1_ALGO_PARAM_ID_TIMESTAMP_2;
    uint32_t param_size = sizeof(dsp_ams_tst_case1_timestamp_2_param_t);

    uint32_t input_port0_ts, output_port0_ts;
    uint32_t input_port1_ts, output_port1_ts;

    uint32_t input0_ts_1st = 0;
    uint32_t input1_ts_1st = 0;
    uint32_t output0_ts_1st = 0;

    uint32_t delta_i0_o0 = 0;
    uint32_t delta_i1_o0 = 0;

    memset( (void*)&cur_ts, 0, sizeof( dsp_ams_tst_case1_timestamp_2_param_t ) );

    rc = ams_get_param(ams_session,
                       pgh,
                       m2.id,
                       pid,
                       &param_size,
                       &cur_ts);

    for(int i = 0; i < DSP_AMS_TST_CASE1_ALGO_MAX_NUM_TS_PAIR; i++)
    {
      input_port0_ts = (uint32_t)cur_ts.port0_ts[i][0];
      output_port0_ts = (uint32_t)cur_ts.port0_ts[i][1];

      if((input_port0_ts != 0) && (input0_ts_1st == 0))
      {
         input0_ts_1st = input_port0_ts;
      }

      if((output_port0_ts != 0) && (output0_ts_1st == 0))
      {
         output0_ts_1st = output_port0_ts;
      }

      printf("input_port0_ts=0x%08X output_port0_ts=0x%08X\n",
               input_port0_ts, output_port0_ts);

      input_port1_ts = (uint32_t)cur_ts.port1_ts[i][0];
      output_port1_ts = (uint32_t)cur_ts.port1_ts[i][1];

      if((input_port1_ts != 0) && (input1_ts_1st == 0))
      {
         input1_ts_1st = input_port1_ts;
      }

      printf("input_port1_ts=0x%08X output_port1_ts=0x%08X\n",
               input_port1_ts, output_port1_ts);

    }

    delta_i0_o0 = output0_ts_1st - input0_ts_1st + 500;
    delta_i1_o0 = output0_ts_1st - input1_ts_1st + 500;

    printf("delay_of_input0_to_output0 =%u us, delay_of_input1_to_output1=%u us\n",
               delta_i0_o0, delta_i1_o0);


    return rc;
}

static int MESSAGE_LINK_BUILD_flag = 0;

static int setup_usecase_graph_1_8155(ams_session_t ams_session, ams_graph_basic_params_t* param, ams_graph_handle_t *gh, uint32_t enable_msg_link)
{
// e2 [SHARE_TDM3_RX, 8 ch, 16 bit] -------------------------------------------> port0|---------|port0-----> e4 {EXCLUSIVE_TDM4_RX, 8 ch, 16 bit}
//                                                                                    |    m1   |
//                                                                                    |         |
// e5 [SHARE_TDM3_TX, 8 ch, 16 bit] ----------->port1|---------|port0----------->port1|         |
//                                                   |    m2   |                      |---------|
// e3 [EXCLUSIVE_TDM4_TX, 8 ch, 16 bit] ------->port0|         |
//                                                   |---------|
  ams_status_t rc = 0;
  ams_endpoint_t e2, e3, e4, e5;
  //ams_module_t m1, m2;
  ams_connection_t c1, c2, c3, c4, c6;
  uint32_t id=1000;
  uint32_t bits=16;
  uint32_t use_mixer_module=1;

  rc = ams_create_graph (ams_session, param, gh);

  // TDM3 RX - shared spkr endpoint
  e2.id = GET_NXT_UNIQUE_ID__(id);
  e2.type = AMS_ENDPOINT_TYPE_SOURCE;
  e2.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT;

  memset( e2.channel_type, 0, sizeof( e2.channel_type ) );
  e2.channel_type[0] = 0;
  e2.channel_type[1] = 1;
  e2.channel_type[2] = 2;
  e2.channel_type[3] = 3;
  e2.channel_type[4] = 4;
  e2.channel_type[5] = 5;
  e2.channel_type[6] = 6;
  e2.channel_type[7] = 7;

  e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
  e2.tdm_params.num_channels = 8;
  e2.tdm_params.bit_width = bits;
  e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
  e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
  e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
  e2.tdm_params.nslots_per_frame = e2.tdm_params.num_channels;
  e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
  e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
  e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
  e2.tdm_params.slot_width = 32;
  e2.tdm_params.slot_mask = (e2.tdm_params.num_channels == 8)? 0xFF : 0xFFFF;
  e2.channel_mask = e2.tdm_params.slot_mask;
  e2.q_factor = (e2.tdm_params.bit_width == 16)? 15:31;

  // TDM4 TX - A2B mic
  e3.id = GET_NXT_UNIQUE_ID__(id);
  e3.type = AMS_ENDPOINT_TYPE_SOURCE;
  e3.flags = AMS_ENDPOINT_EXCLUSIVE;

  memset( e3.channel_type, 0, sizeof( e3.channel_type ) );
  e3.channel_type[0] = 0;
  e3.channel_type[1] = 1;
  e3.channel_type[2] = 2;
  e3.channel_type[3] = 3;
  e3.channel_type[4] = 4;
  e3.channel_type[5] = 5;
  e3.channel_type[6] = 6;
  e3.channel_type[7] = 7;

  e3.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
  e3.tdm_params.num_channels = 8; //16;
  e3.tdm_params.bit_width = bits;
  e3.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
  e3.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
  e3.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
  e3.tdm_params.nslots_per_frame = e3.tdm_params.num_channels;
  e3.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
  e3.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
  e3.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
  e3.tdm_params.slot_width = 32;
  e3.tdm_params.slot_mask = (e3.tdm_params.nslots_per_frame == 8)? 0xFF : 0xFFFF;
  e3.channel_mask = e3.tdm_params.slot_mask;
  e3.q_factor = (e3.tdm_params.bit_width == 16)? 15:31;


  // TDM4 RX - A2B Speaker
  e4.id = GET_NXT_UNIQUE_ID__(id);
  e4.type = AMS_ENDPOINT_TYPE_SINK;
  e4.flags = AMS_ENDPOINT_EXCLUSIVE;

  memset( e4.channel_type, 0, sizeof( e4.channel_type ) );
  e4.channel_type[0] = 0;
  e4.channel_type[1] = 1;
  e4.channel_type[2] = 2;
  e4.channel_type[3] = 3;
  e4.channel_type[4] = 4;
  e4.channel_type[5] = 5;
  e4.channel_type[6] = 6;
  e4.channel_type[7] = 7;

  e4.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
  e4.tdm_params.num_channels = 8; //16;
  e4.tdm_params.bit_width = bits;
  e4.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
  e4.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
  e4.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
  e4.tdm_params.nslots_per_frame = e4.tdm_params.num_channels;
  e4.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
  e4.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
  e4.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
  e4.tdm_params.slot_width = 32;
  e4.tdm_params.slot_mask = (e4.tdm_params.nslots_per_frame == 8)? 0xFF : 0xFFFF;
  e4.channel_mask = e4.tdm_params.slot_mask;
  e4.q_factor = (e4.tdm_params.bit_width == 16)? 15:31;


////
  // TDM3 TX - shared MIC endpoint
  e5.id = GET_NXT_UNIQUE_ID__(id);
  e5.type = AMS_ENDPOINT_TYPE_SOURCE;
  e5.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_INPUT;

  memset( e5.channel_type, 0, sizeof( e5.channel_type ) );
  e5.channel_type[0] = 0;
  e5.channel_type[1] = 1;
  e5.channel_type[2] = 2;
  e5.channel_type[3] = 3;
  e5.channel_type[4] = 4;
  e5.channel_type[5] = 5;
  e5.channel_type[6] = 6;
  e5.channel_type[7] = 7;

  e5.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
  e5.tdm_params.num_channels = 8;
  e5.tdm_params.bit_width = bits;
  e5.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
  e5.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
  e5.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
  e5.tdm_params.nslots_per_frame = e5.tdm_params.num_channels;
  e5.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
  e5.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
  e5.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
  e5.tdm_params.slot_width = 32;
  e5.tdm_params.slot_mask = (e5.tdm_params.num_channels == 8)? 0xFF : 0xFFFF;
  e5.channel_mask = e5.tdm_params.slot_mask;
  e5.q_factor = (e5.tdm_params.bit_width == 16)? 15:31;

  m1.id = GET_NXT_UNIQUE_ID__(id);
  m1.flags = AMS_INLINE_PROCESSING_MODE;
  if (use_mixer_module) {
    m1.capiv2_info.id = CAPI_MODULE_ID_MIXER;
  } else {
    m1.capiv2_info.id = 0x123E0000;   //CAPI_V2_ANC_MODULE_ID
  }
  strlcpy (m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
  strlcpy (m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

  m2.id = GET_NXT_UNIQUE_ID__(id);
  m2.flags = AMS_INLINE_PROCESSING_MODE;
  m2.capiv2_info.id = DSP_AMS_TST_CASE1_ALGO_MODULE_ID;
  strlcpy (m2.capiv2_info.tag, "", sizeof(m2.capiv2_info.tag));
  strlcpy (m2.capiv2_info.shared_obj_filename, "", sizeof(m2.capiv2_info.shared_obj_filename));

  // from shared endpoint TDM3 RX to M1 module
  c1.num_channels = e2.tdm_params.num_channels;
  c1.bit_width = e2.tdm_params.bit_width;
  c1.source.type =  AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
  c1.source.endpoint.id = e2.id;
  c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
  c1.destination.module.id = m1.id;
  c1.destination.module.port_index = 0;

  // from source endpoint TDM4 TX to M2 module, input_port_0
  c2.num_channels = e3.tdm_params.num_channels;
  c2.bit_width = e3.tdm_params.bit_width;
  c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
  c2.source.endpoint.id = e3.id;
  c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
  c2.destination.module.id = m2.id;
  c2.destination.module.port_index = 0;

  //from m1 to TDM4_RX
  c3.num_channels = e4.tdm_params.num_channels;
  c3.bit_width = e4.tdm_params.bit_width;
  c3.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
  c3.source.module.id = m1.id;
  c3.source.module.port_index = 0;
  c3.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
  c3.destination.endpoint.id = e4.id;

  //FROM M2 TO M1
  c4.num_channels = 8;
  c4.bit_width = bits;
  c4.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
  c4.source.module.id = m2.id;
  c4.source.module.port_index = 0;
  c4.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
  c4.destination.module.id = m1.id;
  c4.destination.module.port_index = 1;

  // from SHARE_MIC TERT TDM TX to M2 module, input_port_1
  c6.num_channels = e5.tdm_params.num_channels;
  c6.bit_width = e5.tdm_params.bit_width;
  c6.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
  c6.source.endpoint.id = e5.id;
  c6.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
  c6.destination.module.id = m2.id;
  c6.destination.module.port_index = 1;
///

    rc = ams_graph_add_endpont (ams_session, *gh, &e4);
    rc = ams_graph_add_endpont (ams_session, *gh, &e2);
    rc = ams_graph_add_endpont (ams_session, *gh, &e3);
    rc = ams_graph_add_endpont (ams_session, *gh, &e5);
    rc = ams_graph_add_module (ams_session, *gh, &m1);
    rc = ams_graph_add_module (ams_session, *gh, &m2);
    rc = ams_graph_add_connection (ams_session, *gh, &c1);
    rc = ams_graph_add_connection (ams_session, *gh, &c2);
    rc = ams_graph_add_connection (ams_session, *gh, &c3);
    rc = ams_graph_add_connection (ams_session, *gh, &c4);
    rc = ams_graph_add_connection (ams_session, *gh, &c6);
    rc = ams_util_print_graph_info (ams_session, *gh);

    rc = ams_open_graph (ams_session, *gh);

    printf("ams_service: open graph returned value 0x%x",rc);
    if(*gh == NULL)
    {
        printf("ams_service: Unable to open graph, exiting ams service");
        return rc;
    }

    if (use_mixer_module) {
        // configure mixer module
        #define NUM_TAPS 16
        #define UNITY 0x8000000
        #define HALF  0x4000000
        struct {
        uint32_t num_taps;
        capi_mixer_tap_t taps[NUM_TAPS];
        } mixer_conf = {
        NUM_TAPS,
        {
        /*inport, inch, outport, outch, gain Q27 */
            { 0,  0, 0,  0, HALF},
            { 1,  0, 0,  0, HALF},

            { 0,  1, 0,  1, HALF},
            { 1,  1, 0,  1, HALF},

            { 0,  2, 0,  2, HALF},
            { 1,  2, 0,  2, HALF},

            { 0,  3, 0,  3, HALF},
            { 1,  3, 0,  3, HALF},

            { 0,  4, 0,  4, HALF},
            { 1,  4, 0,  4, HALF},

            { 0,  5, 0,  5, HALF},
            { 1,  5, 0,  5, HALF},

            { 0,  6, 0,  6, HALF},
            { 1,  6, 0,  6, HALF},

            { 0,  7, 0,  7, HALF},
            { 1,  7, 0,  7, HALF},
        }};
        rc = ams_set_param(ams_session, *gh, m1.id,
        CAPI_PARAM_ID_MIXER_CONFIG, sizeof(mixer_conf), &mixer_conf);
        printf("ams_test: set mixer config rc=%d mixer_cfg_size=%d\n", rc, sizeof(mixer_conf));
    }
    ams_msg_link_connection_t msg_link_conn[2];
    ams_graph_property_t msg_prop;
    if (enable_msg_link)
    {
    //setup TIMESTAMP MSG link
    // set graph property test

        //e3 [EXCLUSIVE_TDM4_TX]------>input_port0 m2
        msg_link_conn[0].source.hw_interface_id = AMS_HW_INTERFACE_TDM4;
        msg_link_conn[0].source.type = AMS_ENDPOINT_TYPE_SOURCE;
        msg_link_conn[0].destination.id = m2.id;
        msg_link_conn[0].destination.module_id = m2.capiv2_info.id;
        msg_link_conn[0].destination.port_type = AMS_MODULE_INPUT_PORT;
        msg_link_conn[0].destination.port_index = 0;

        //e4 {EXCLUSIVE_TDM4_RX}--------->output_port0 m2
        msg_link_conn[1].source.hw_interface_id = AMS_HW_INTERFACE_TDM4;
        msg_link_conn[1].source.type = AMS_ENDPOINT_TYPE_SINK;
        msg_link_conn[1].destination.id = m2.id;
        msg_link_conn[1].destination.module_id = m2.capiv2_info.id;
        msg_link_conn[1].destination.port_type = AMS_MODULE_OUTPUT_PORT;
        msg_link_conn[1].destination.port_index = 0;

        msg_prop.prop_id = AMS_GRAPH_PROPERTY_ID_TIMESTAMP_MESSAGE_LINK_BUILD;
        msg_prop.u.ams_build_msg_link.num_link = 2;
        msg_prop.u.ams_build_msg_link.msg_link_ptr = &msg_link_conn[0];
        msg_prop.appy_after_start = 0;
        ams_status_t rc = ams_graph_set_property(ams_session, *gh, &msg_prop);

        MESSAGE_LINK_BUILD_flag = 1;

        printf("ams_test: set graph property for MESSAGE_LINK_BUILD %d  returned value 0x%x\n", msg_prop.prop_id, rc);
    }

    if (1) {
        // set graph property test
        ams_graph_property_t prop;
        prop.prop_id = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_CLK_ATTR;
        prop.u.exclv_ep_clk_attr.exclv_ep_id = AMS_HW_INTERFACE_TDM4;
        prop.u.exclv_ep_clk_attr.clk_invert = 4;
        prop.appy_after_start = 0;
        ams_status_t rc = ams_graph_set_property(ams_session, *gh, &prop);
        printf("ams_test: set graph property for %d returned value 0x%x\n", prop.u.exclv_ep_clk_attr.exclv_ep_id, rc);

        /*
        prop.u.exclv_ep_clk_attr.exclv_ep_id = AMS_HW_INTERFACE_TDM5;
        rc = ams_graph_set_property(ams_session, *gh, &prop);
        printf("ams_test: set graph property for %d  returned value 0x%x\n", prop.u.exclv_ep_clk_attr.exclv_ep_id, rc);
        */
    }

    rc = ams_start_graph (ams_session, *gh);
    //printf("ams_service: start graph returned value 0x%x",rc);

    return rc;
}

static int setup_usecase_graph_mixer_8155(ams_session_t ams_session, ams_graph_basic_params_t* param, ams_graph_handle_t *gh)
{
    // e2 [SHARE_TDM3_RX, 8 ch, 32 bit] --> port0|---------|port0-----> e4 {EXCLUSIVE_TDM4_RX, 16 ch, 32 bit}
    //                                           |    m1   |
    //                                           |         |
    // e5 [SHARE_TDM3_TX, 8 ch, 32 bit] --->port1|         |
    //                                           |---------|

    ams_status_t rc = 0;
    ams_endpoint_t e2, e4, e5;
    //ams_module_t m1, m2;
    ams_connection_t c1, c3, c6;
    uint32_t id=1000;
    uint32_t bits = 16;

    rc = ams_create_graph (ams_session, param, gh);

    // TDM3 RX - shared spkr endpoint
    e2.id = GET_NXT_UNIQUE_ID__(id);
    e2.type = AMS_ENDPOINT_TYPE_SOURCE;
    e2.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT;

    memset( e2.channel_type, 0, sizeof( e2.channel_type ) );
    e2.channel_type[0] = 0;
    e2.channel_type[1] = 1;
    e2.channel_type[2] = 2;
    e2.channel_type[3] = 3;
    e2.channel_type[4] = 4;
    e2.channel_type[5] = 5;
    e2.channel_type[6] = 6;
    e2.channel_type[7] = 7;

    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
    e2.tdm_params.num_channels = 8;
    e2.tdm_params.bit_width = bits;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = e2.tdm_params.num_channels;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = (e2.tdm_params.num_channels == 8)? 0xFF : 0xFFFF;
    e2.channel_mask = e2.tdm_params.slot_mask;
    e2.q_factor = (e2.tdm_params.bit_width == 16)? 15:31;

    // TDM4 RX - A2B Speaker
    e4.id = GET_NXT_UNIQUE_ID__(id);
    e4.type = AMS_ENDPOINT_TYPE_SINK;
    e4.flags = AMS_ENDPOINT_EXCLUSIVE;

    memset( e4.channel_type, 0, sizeof( e4.channel_type ) );
    e4.channel_type[0] = 0;
    e4.channel_type[1] = 1;
    e4.channel_type[2] = 2;
    e4.channel_type[3] = 3;
    e4.channel_type[4] = 4;
    e4.channel_type[5] = 5;
    e4.channel_type[6] = 6;
    e4.channel_type[7] = 7;

    e4.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    e4.tdm_params.num_channels = 16;
    e4.tdm_params.bit_width = bits;
    e4.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e4.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e4.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e4.tdm_params.nslots_per_frame = e4.tdm_params.num_channels;
    e4.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e4.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e4.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e4.tdm_params.slot_width = 32;
    e4.tdm_params.slot_mask = (e4.tdm_params.num_channels == 8)? 0xFF : 0xFFFF;
    e4.channel_mask = e4.tdm_params.slot_mask;
    e4.q_factor = (e4.tdm_params.bit_width == 16)? 15:31;

    // TDM3 TX - shared MIC endpoint
    e5.id = GET_NXT_UNIQUE_ID__(id);
    e5.type = AMS_ENDPOINT_TYPE_SOURCE;
    e5.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_INPUT;

    memset( e5.channel_type, 0, sizeof( e5.channel_type ) );
    e5.channel_type[0] = 0;
    e5.channel_type[1] = 1;
    e5.channel_type[2] = 2;
    e5.channel_type[3] = 3;
    e5.channel_type[4] = 4;
    e5.channel_type[5] = 5;
    e5.channel_type[6] = 6;
    e5.channel_type[7] = 7;

    e5.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
    e5.tdm_params.num_channels = 8;
    e5.tdm_params.bit_width = bits;
    e5.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e5.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e5.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e5.tdm_params.nslots_per_frame = e5.tdm_params.num_channels;
    e5.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e5.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e5.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e5.tdm_params.slot_width = 32;
    e5.tdm_params.slot_mask = (e5.tdm_params.num_channels == 8)? 0xFF : 0xFFFF;
    e5.channel_mask = e5.tdm_params.slot_mask;
    e5.q_factor = (e5.tdm_params.bit_width == 16)? 15:31;

    m1.id = GET_NXT_UNIQUE_ID__(id);
    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = CAPI_MODULE_ID_MIXER;
    strlcpy (m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy (m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

    // from shared endpoint TDM3 RX to M1 module
    c1.num_channels = e2.tdm_params.num_channels;
    c1.bit_width = e2.tdm_params.bit_width;
    c1.source.type =  AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.source.endpoint.id = e2.id;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.destination.module.id = m1.id;
    c1.destination.module.port_index = 0;

    //from m1 to TDM4_RX
    c3.num_channels = e4.tdm_params.num_channels;
    c3.bit_width = e4.tdm_params.bit_width;
    c3.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c3.source.module.id = m1.id;
    c3.source.module.port_index = 0;
    c3.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c3.destination.endpoint.id = e4.id;

    // from SHARE_MIC TERT TDM TX to M1 module, input_port_1
    c6.num_channels = e5.tdm_params.num_channels;
    c6.bit_width = e5.tdm_params.bit_width;
    c6.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c6.source.endpoint.id = e5.id;
    c6.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c6.destination.module.id = m1.id;
    c6.destination.module.port_index = 1;
    ///

    rc = ams_graph_add_endpont (ams_session, *gh, &e4);
    rc = ams_graph_add_endpont (ams_session, *gh, &e2);
    rc = ams_graph_add_endpont (ams_session, *gh, &e5);
    rc = ams_graph_add_module (ams_session, *gh, &m1);
    rc = ams_graph_add_connection (ams_session, *gh, &c1);
    rc = ams_graph_add_connection (ams_session, *gh, &c3);
    rc = ams_graph_add_connection (ams_session, *gh, &c6);
    rc = ams_util_print_graph_info (ams_session, *gh);

    rc = ams_open_graph (ams_session, *gh);

    //printf("ams_service: open graph returned value 0x%x",rc);
    if(*gh == NULL)
    {
        printf("ams_service: Unable to open graph, exiting ams service");
        return rc;
    }

    // configure mixer module
    #define NUM_TAPS 12
    #define UNITY 0x8000000
    #define HALF  0x4000000
    struct {
        uint32_t num_taps;
        capi_mixer_tap_t taps[NUM_TAPS];
    } mixer_conf = {
        NUM_TAPS,
        {
        /*inport, inch, outport, outch, gain Q27 */
        { 0,  0, 0,  0, UNITY},
        { 0,  1, 0,  1, UNITY},

        { 1,  0, 0,  2, UNITY},
        { 1,  1, 0,  3, UNITY},

        { 0,  0, 0,  4, HALF},
        { 1,  0, 0,  4, HALF},

        { 0,  1, 0,  5, HALF},
        { 1,  1, 0,  5, HALF},

        { 0,  2, 0,  6, HALF},
        { 1,  2, 0,  6, HALF},

        { 0,  3, 0,  7, HALF},
        { 1,  3, 0,  7, HALF},
    }};
    printf("mixer config:\n");
    printf("inport, inch, outport, outch, gain Q27\n");
    printf("  { 0,  0, 0,  0, UNITY}\n");
    printf("  { 0,  1, 0,  1, UNITY}\n");
    printf("  { 1,  0, 0,  2, UNITY}\n");
    printf("  { 1,  1, 0,  3, UNITY}\n");
    printf("  { 0,  0, 0,  4, HALF}\n");
    printf("  { 1,  0, 0,  4, HALF}\n");
    printf("  { 0,  1, 0,  5, HALF}\n");
    printf("  { 1,  1, 0,  5, HALF}\n");
    printf("  { 0,  2, 0,  6, HALF}\n");
    printf("  { 1,  2, 0,  6, HALF}\n");
    printf("  { 0,  3, 0,  7, HALF}\n");
    printf("  { 1,  3, 0,  7, HALF}\n");
    rc = ams_set_param(ams_session, *gh, m1.id,
        CAPI_PARAM_ID_MIXER_CONFIG, sizeof(mixer_conf), &mixer_conf);
    printf("ams_test: set mixer config rc=%d mixer_cfg_size=%d\n", rc, sizeof(mixer_conf));

    rc = ams_start_graph (ams_session, *gh);
    //printf("ams_service: start graph returned value 0x%x",rc);

    return rc;
}

static int setup_usecase_msg_link_8155(ams_session_t ams_session, ams_graph_basic_params_t* param, ams_graph_handle_t *gh)
{
    return setup_usecase_graph_1_8155(ams_session, param, gh, 1);
}

static int setup_usecase_share_mic_spkr_8155(ams_session_t ams_session, ams_graph_basic_params_t* param, ams_graph_handle_t *gh)
{
    return setup_usecase_graph_1_8155(ams_session, param, gh, 0);
}

static int teardown_usecase(ams_session_t ams_session, ams_graph_handle_t g_handle)
{
  ams_status_t rc = 0;

  if(MESSAGE_LINK_BUILD_flag)
  {
     //destroy
     // set graph property test
     ams_graph_property_t prop = {0};
    prop.prop_id = AMS_GRAPH_PROPERTY_ID_TIMESTAMP_MESSAGE_LINK_DESTROY;
    prop.appy_after_start = 3;
    ams_status_t rc = ams_graph_set_property(ams_session, g_handle, &prop);
    printf("ams_test: set graph property for MESSAGE_LINK_DESTROY %d returned value 0x%x\n", prop.prop_id, rc);

    MESSAGE_LINK_BUILD_flag = 0;
  }

    rc = ams_stop_graph(ams_session, g_handle);

    rc = ams_close_graph(ams_session, g_handle);
    rc = ams_destroy_graph(ams_session, &g_handle);

    g_handle = NULL;

    return rc;
}

static int show_options(void)
{
    printf("ams_test_II: AMS Lib test app options\n");
    printf("1: Show test opt\n");
    printf("2: start_usecase_share_mic_spkr_6155\n");
    printf("8: start_usecase_share_mic_spkr_8155 or 8195\n");
    printf("3: setup_usecase_share_mic_spkr, set input port_0 - from EXCLUSIVE SEC_TDM_TX\n");
    printf("4: setup_usecase_share_mic_spkr, set input port_1 - from SHARE_MIC\n");
    printf("5: setup_usecase_share_mic_spkr, get timestamp \n");
    printf("6: Stop & close graph\n");
    printf("7: quit this test\n");

    printf("9: start_usecase_msg_link_6155\n");
    printf("a: start_usecase_msg_link_8155 or 8195\n");
    printf("b: usecase_msg_link, get timestamp \n");
    printf("c: setup_usecase_graph_mixer_8155\n");
    return 0;
}


void share_mic_test(void)
{
    char c = 0;
    char cmd_str[4]="";
    ams_status_t rc = 0;

    g_param.flags = 0;
    g_param.block_size = 12;
    g_param.processor_id = 0;
    g_param.sample_rate = 48000;
    rc = ams_init(&ams_session);
    printf("ams_test_load: init returned value : 0x%x\n", rc);

    show_options();
    while (1)
    {
        fgets(cmd_str, 4, stdin);
        c = cmd_str[0];
        switch (c)
        {
        case '1':
            rc = show_options();
            break;
        case '2':
          //setup_usecase_share_mic_spkr_6155
          printf("setup_usecase_share_mic_spkr_6155 not implemented\n");
          rc = 1;
          //my_TDM5_CFG_FLAG = 0;
          //rc = setup_usecase_share_mic_spkr_6155(ams_session, &g_param, &g_handle1);
          break;

        case '3':
          //setup_usecase_share_mic_spkr_6155, set input port0
            rc = ams_set_input_port_share_mic_spkr(0);
            break;
        case '4':
          //setup_usecase_share_mic_spkr_6155, set input port1
            rc = ams_set_input_port_share_mic_spkr(1);
            break;

        case '5':
          //ams_get_timestamp_share_mic_spkr
          rc = ams_get_timestamp_share_mic_spkr();
          break;

        case '6':
          //5: Stop & close graph
          rc = teardown_usecase(ams_session, g_handle1);
          break;

        case '7':
          printf("Quit\n");
              goto quit;

        case '8':
          printf("setup_usecase_share_mic_spkr_8155\n");
          rc = setup_usecase_share_mic_spkr_8155(ams_session, &g_param, &g_handle1);
          //my_TDM5_CFG_FLAG = 1;
          break;
        case '9':
          printf("start_usecase_msg_link_6155 not implemented\n");
          rc = 1;
          break;
        case 'a':
          printf("start_usecase_msg_link_8155\n");
          rc = setup_usecase_msg_link_8155(ams_session, &g_param, &g_handle1);
            break;

        case 'b':
          printf("usecase_msg_link, get timestamp\n");
          rc = usecase_msg_link_get_timestamp();
            break;

        case 'c':
          printf("setup_usecase_graph_mixer_8155\n");
          rc = setup_usecase_graph_mixer_8155(ams_session, &g_param, &g_handle1);
          break;

        default:
            rc = 1;
            break;
        }
        printf("%s\n", (rc == 0 ? "OK" : "NOK"));
        fflush(stdout);
    }

    quit:
    rc = ams_deinit(&ams_session);
    printf("ams_test: deinit returned value : 0x%x\n", rc);
    printf("Close app %s\n", (rc == 0 ? "OK" : "NOK"));
}
