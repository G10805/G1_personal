/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_test"
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

#define AMS_TEST_UC_MIN 1
#define AMS_TEST_UC_MAX 2
#define AMS_MULTILANE_TEST_UC_MAX 5

#define AMS_TEST_TARGET_6150 1
#define AMS_TEST_TARGET_8150 2

static ams_session_t ams_session = NULL;
static ams_graph_basic_params_t g_param;
static ams_graph_handle_t g_handles[AMS_TEST_UC_MAX] = {NULL};
static ams_graph_handle_t g_mlt_handles[AMS_MULTILANE_TEST_UC_MAX] = {NULL};
static int ams_usecase = 0;
static uint32_t ams_test_gid = 0;
static uint32_t ams_test_target_id = AMS_TEST_TARGET_8150;

#define AMS_OPERATE_DEV_NUM_MAX 5
static struct ams_operate_dev
{
    uint32_t dev_id;
    uint32_t en;
} ams_operate_dev[AMS_OPERATE_DEV_NUM_MAX] = {0};

#define AMS_OPERATE_PORT_NUM_MAX 5
static struct ams_operate_port
{
    uint32_t port_id;
    uint32_t en;
} ams_operate_port[AMS_OPERATE_PORT_NUM_MAX] = {0};

#define GET_NXT_UNIQUE_ID(x) (++x)
#define AMS_DUMMY_MODULE_ID 0x00013378
#define AMS_DUMNMY_PARAM_ID 0x10000002
#define AMS_DUMMY_PARAM_DATA (0x12345678)
#define AMS_DUMMY_PARAM_SIZE (sizeof(AMS_DUMMY_PARAM_DATA))

#define CAPI_V2_MODULE_ID_LOAD                  0x070010B9

typedef enum AmsTestMsgLevel
{
    AmsTestMsgLevel_Unused = 0,
    AmsTestMsgLevel_Low,
    AmsTestMsgLevel_Med,
    AmsTestMsgLevel_High,
    AmsTestMsgLevel_Error
} AmsTestMsgLevel;

static AmsTestMsgLevel g_AmsTestMsgLevel = AmsTestMsgLevel_Low;
static uint32_t ams_test_delay_mid;

void ams_ssr_user_cb_func(
    ams_event_id_t e,
    void *data)
{
    ams_status_t r = 0;
    ams_graph_handle_t *pgh = data;
    int i;
    switch (e)
    {
    case AMS_EVENT_SSR_STARTED:
        AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "AMS SSR callback => SSR_EVENT_RESTART_START");
        break;
    case AMS_EVENT_SSR_COMPLETED:
        AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "AMS SSR callback => SSR_EVENT_RESTART_COMPLETE");
        // Open and start graph from testapp after SSR is complete.
        // for (i = 0; i < AMS_TEST_UC_MAX; i++)
        // {
            // if (pgh[i])
            // {
                // r = ams_open_graph(ams_session, pgh[i]);
                // AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "ams_test:SSR_COMPLETED: reopen graph(0x%lx) UC %d returned value : 0x%x\n", (uintptr_t)pgh[i], i + 1, r);
                // if (!r)
                // {
                //    r = ams_start_graph(ams_session, pgh[i]);
                //    AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "ams_test:SSR_COMPLETED: restart graph(0x%lx) UC %d returned value : 0x%x\n", (uintptr_t)pgh[i], i + 1, r);
                //}
            // }
        //}
        break;
    default:
        AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "AMS SSR callback => default event received");
        break;
    }
}

static int show_options(void)
{
    printf("ams_test: AMS test app options\n");
    printf("0: Show test opt\n");
    printf("1: Set usecase (1: TDM4=>M(delay)=>TDM3, 2:TDM3=>=>TDM4)\n");
    printf("2: Create graph\n");
    printf("3: Open graph\n");
    printf("4: Start graph \n");
    printf("5: Set delay param (1: in-band, 2: out-band)\n");
    printf("6: Start playback\n");
    printf("7: Stop graph\n");
    printf("8: Close graph\n");
    printf("9: Destroy graph\n");
    printf("a: go to share_mic test case\n");
    printf("m: Multilane TDM usecase\n");
    printf("p: TDM port endable/disable\n");
    printf("s: SSR Test\n");
    printf("q: Quit\n");
    return 0;
}

static int set_usecase(int usecase)
{
    ams_usecase = usecase;
    return 0;
}

static ams_graph_handle_t *usecase_get_graph_phdl(int uc)
{
    ams_graph_handle_t *pgh;

    if (uc > AMS_TEST_UC_MAX || uc < AMS_TEST_UC_MIN)
    {
        pgh = NULL;
    }
    else
    {
        pgh = &g_handles[uc - 1];
    }
    return pgh;
}

static int create_graph_1(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh)
{
    printf("Start ams_test for graph 1\n");
    printf("e2 {TDM4 (SHARE_EP 32 ch, 16 bit)} --c2--> M(mixer) --c1--> e1 {TDM3 (EXCLUSIVE EP 8 ch, 16 bit)}\n");

    ams_status_t rc = 0;
    ams_endpoint_t e1, e2;
    ams_module_t m1;
    ams_connection_t c1, c2;
    int i = 0;

    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    memset(&m1, 0, sizeof(m1));
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));

    rc = ams_create_graph(ams_session, param, pgh);

    // TDM3 exclusive 8ch/16bit
    e1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e1.type = AMS_ENDPOINT_TYPE_SINK;
    e1.channel_mask = 0xFF;
    e1.q_factor = 15;
    e1.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e1.channel_type, 0, sizeof(e1.channel_type));
    for (i = 0; i < 8; i++) {
        e1.channel_type[i] = i;
    }

    e1.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
    e1.tdm_params.num_channels = 8;
    e1.tdm_params.bit_width = 16;
    e1.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e1.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e1.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e1.tdm_params.nslots_per_frame = 8;
    e1.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e1.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e1.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e1.tdm_params.slot_width = 32;
    e1.tdm_params.slot_mask = 0xFF;

    // TDM4 shared ep
    e2.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e2.type = AMS_ENDPOINT_TYPE_SOURCE;
    e2.channel_mask = 0xFFFF;
    e2.q_factor = 15;
    e2.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT;
    AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "AMS Test: asizeof( e2.channel_type ) %lu", sizeof(e2.channel_type));

    memset(e2.channel_type, 0, sizeof(e2.channel_type));
    for (i = 0; i < 32; i++) {
        e2.channel_type[i] = i;
    }


    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    e2.tdm_params.num_channels = 32;
    e2.tdm_params.bit_width = 16;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = 16;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = 0xFFFF;

    m1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    ams_test_delay_mid = m1.id;

    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = CAPI_MODULE_ID_MIXER; //0x123A8000
    strlcpy(m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

#if 0
    // dynamic load module
    m1.capiv2_info.id=CAPI_V2_MODULE_ID_LOAD;
    strlcpy(m1.capiv2_info.tag, "capi_load", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "load_lib_dd.so", sizeof(m1.capiv2_info.shared_obj_filename));
#endif

    c1.num_channels = 8;
    c1.bit_width = e1.tdm_params.bit_width;
    c1.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.source.module.id = m1.id;
    c1.source.module.port_index = 0;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.destination.endpoint.id = e1.id;

    c2.num_channels = 32;
    c2.bit_width = e2.tdm_params.bit_width;
    c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c2.source.endpoint.id = e2.id;
    c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c2.destination.module.id = m1.id;
    c2.destination.module.port_index = 0;

    rc = ams_graph_add_endpont(ams_session, *pgh, &e1);
    rc |= ams_graph_add_endpont(ams_session, *pgh, &e2);
    rc |= ams_graph_add_module(ams_session, *pgh, &m1);
    rc |= ams_graph_add_connection(ams_session, *pgh, &c1);
    rc |= ams_graph_add_connection(ams_session, *pgh, &c2);
    rc |= ams_util_print_graph_info(ams_session, *pgh);

#if 0
    {
        // set graph property
        ams_graph_property_t prop;
        prop.prop_id = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_CLK_ATTR;
        prop.u.exclv_ep_clk_attr.exclv_ep_id = AMS_HW_INTERFACE_TDM3;
        prop.u.exclv_ep_clk_attr.clk_invert = 4;
        prop.appy_after_start = 0;
        ams_status_t rc = ams_graph_set_property(ams_session, *pgh, &prop);
        printf("ams_test: set graph property for %d returned value 0x%x\n", prop.u.exclv_ep_clk_attr.exclv_ep_id, rc);
        prop.u.exclv_ep_clk_attr.exclv_ep_id = AMS_HW_INTERFACE_TDM5;
        rc = ams_graph_set_property(ams_session, *pgh, &prop);
        printf("ams_test: set graph property for %d  returned value 0x%x\n", prop.u.exclv_ep_clk_attr.exclv_ep_id, rc);
    }
#endif


    // configure mixer module
    #define NUM_TAPS 8
    #define UNITY 0x8000000
    #define HALF  0x4000000
    struct {
        uint32_t num_taps;
        capi_mixer_tap_t taps[NUM_TAPS];
    } mixer_conf = {
        NUM_TAPS,
        {
        /*inport, inch, outport, outch, gain Q27 */
        { 0,   8, 0,  0, UNITY},
        { 0,   9, 0,  1, UNITY},
        { 0,  12, 0,  2, UNITY},
        { 0,  13, 0,  3, UNITY},
        { 0,   0, 0,  4, UNITY},
        { 0,   1, 0,  5, UNITY},
        { 0,   4, 0,  6, UNITY},
        { 0,   5, 0,  7, UNITY},
    }};
    printf("mixer config:\n");
    printf("inport, inch, outport, outch, gain Q27\n");
    printf("  { 0,   8, 0,  0, UNITY}\n");
    printf("  { 0,   9, 0,  1, UNITY}\n");
    printf("  { 0,  12, 0,  2, UNITY}\n");
    printf("  { 0,  13, 0,  3, UNITY}\n");
    printf("  { 0,   0, 0,  4, UNITY}\n");
    printf("  { 0,   1, 0,  5, UNITY}\n");
    printf("  { 0,   4, 0,  6, UNITY}\n");
    printf("  { 0,   5, 0,  7, UNITY}\n");

    rc = ams_open_graph(ams_session, *pgh);
    printf("ams_open_graph returned %d\n", rc);
    rc = ams_set_param(ams_session, *pgh, m1.id,
        CAPI_PARAM_ID_MIXER_CONFIG, sizeof(mixer_conf), &mixer_conf);
    printf("ams_test: set mixer config rc=%d mixer_cfg_size=%d\n", rc, sizeof(mixer_conf));

    return rc;
}


static int create_graph_2(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh)
{
    ams_status_t rc = 0;
    ams_endpoint_t e1, e2;
    ams_module_t m1;
    ams_connection_t c1, c2;

    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    memset(&m1, 0, sizeof(m1));
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));

    rc = ams_create_graph(ams_session, param, pgh);
    // source
    e1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e1.type = AMS_ENDPOINT_TYPE_SOURCE;
    e1.channel_mask = 0xFF;
    e1.q_factor = 31;
    e1.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT;

    AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "AMS Test: asizeof( e1.channel_type ) %lu", sizeof(e1.channel_type));

    memset(e1.channel_type, 0, sizeof(e1.channel_type));
    e1.channel_type[0] = 0;
    e1.channel_type[1] = 1;
    e1.channel_type[2] = 2;
    e1.channel_type[3] = 3;
    e1.channel_type[4] = 4;
    e1.channel_type[5] = 5;
    e1.channel_type[6] = 6;
    e1.channel_type[7] = 7;

    e1.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
    e1.tdm_params.num_channels = 8;
    e1.tdm_params.bit_width = 16;
    e1.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e1.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e1.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e1.tdm_params.nslots_per_frame = 8;
    e1.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e1.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e1.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e1.tdm_params.slot_width = 32;
    e1.tdm_params.slot_mask = 0xFF;
    // sink
    e2.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e2.type = AMS_ENDPOINT_TYPE_SINK;
    e2.channel_mask = 0xFF;
    e2.q_factor = 31;
    e2.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e2.channel_type, 0, sizeof(e2.channel_type));
    e2.channel_type[0] = 0;
    e2.channel_type[1] = 1;
    e2.channel_type[2] = 2;
    e2.channel_type[3] = 3;
    e2.channel_type[4] = 4;
    e2.channel_type[5] = 5;
    e2.channel_type[6] = 6;
    e2.channel_type[7] = 7;

    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    e2.tdm_params.num_channels = 8;
    e2.tdm_params.bit_width = 16;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = 8;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = 0xFF;

    m1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    ams_test_delay_mid = m1.id;
    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = 0x00013375;
    // m1.capiv2_info.id = 0x123E0000;
    strlcpy(m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

#if 0
    printf("load load_lib_dd.so\n");
    // dynamic load module
    m1.capiv2_info.id=CAPI_V2_MODULE_ID_LOAD;
    strlcpy(m1.capiv2_info.tag, "capi_load", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "load_lib_dd.so", sizeof(m1.capiv2_info.shared_obj_filename));
#endif

    c1.num_channels = 8;
    c1.bit_width = 16;
    c1.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.source.endpoint.id = e1.id;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.destination.module.id = m1.id;
    c1.destination.module.port_index = 0;

    c2.num_channels = 8;
    c2.bit_width = 16;
    c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c2.source.module.id = m1.id;
    c2.source.module.port_index = 0;
    c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c2.destination.endpoint.id = e2.id;

    rc = ams_graph_add_endpont(ams_session, *pgh, &e2);

    rc |= ams_graph_add_endpont(ams_session, *pgh, &e1);

    rc |= ams_graph_add_module(ams_session, *pgh, &m1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c2);

    rc |= ams_util_print_graph_info(ams_session, *pgh);

    return rc;
}

typedef int (*create_gr_func_t)(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh);
static create_gr_func_t create_gr_arr[AMS_TEST_UC_MAX+1] = {
    &create_graph_1,
    &create_graph_2,
	NULL};

static int open_graph(ams_session_t ams_session, ams_graph_handle_t gh)
{
    int rc = ams_open_graph(ams_session, gh);
    printf("ams_test: open graph returned value 0x%x\n", rc);
    return rc;
}

static int start_graph(ams_session_t ams_session, ams_graph_handle_t gh)
{
    int rc = ams_start_graph(ams_session, gh);
    printf("ams_test: start graph returned value 0x%x\n", rc);
    return rc;
}

static int stop_graph(ams_session_t ams_session, ams_graph_handle_t gh)
{
    int rc = ams_stop_graph(ams_session, gh);
    printf("ams_test: stop graph returned value 0x%x\n", rc);
    return rc;
}

static int close_graph(ams_session_t ams_session, ams_graph_handle_t gh)
{
    int rc = ams_close_graph(ams_session, gh);
    printf("ams_test: close graph returned value 0x%x\n", rc);
    return rc;
}

static int destroy_graph(ams_session_t ams_session, ams_graph_handle_t *pgh)
{
    int rc = ams_destroy_graph(ams_session, pgh);
    printf("ams_test: destroy graph returned value 0x%x\n", rc);
    return rc;
}

static int start_playback(void)
{
    int rc = system("audio_chime -d 106 /etc/car2-chime.wav&");
    printf("audio_chime -d 106 returned value 0x%x\n", rc);
    return rc;
}

static int close_app(void)
{
    int i;
    ams_graph_handle_t *pgh;
    int rc;
    for (i = 0; i < AMS_TEST_UC_MAX; i++)
    {
        pgh = &g_handles[i];
        if (*pgh)
        {
            rc = ams_destroy_graph(ams_session, pgh);
            printf("ams_test: destroy graph UC %d returned value : 0x%x\n", i, rc);
        }
    }
    rc = ams_deinit(&ams_session);
    printf("ams_test: deinit returned value : 0x%x\n", rc);
    printf("ams_test: Exiting...\n");
    return 0;
}

#define AMS_TEST_KB_NUM 16
static int set_delay_param(int type)
{
    int rc;
    ams_graph_handle_t *pgh = usecase_get_graph_phdl(ams_usecase);
    if (pgh == NULL)
        return AMS_STATUS_GENERAL_ERROR;
    int8_t data[AMS_TEST_KB_NUM * 1024];
    uint32_t *delay;
    memset(data, 0, AMS_TEST_KB_NUM * 1024);
#if 1
    {
        if (type == 1)
        {
            // set param test - inband
            dsp_ams_delay_param_t delay_param;
            delay_param.delay_us = 1000;

            rc = ams_set_param(ams_session, *pgh, ams_test_delay_mid, DSP_AMS_PARAM_ID_DELAY, sizeof(dsp_ams_delay_param_t), &delay_param);
            printf("ams_test: ams_set_param in-band result %d\n", rc);
        }
        else if (type == 2)
        {
            // set param test - outband
            delay = (uint32_t *)data;
            *delay = 1000;
            printf("ams_test: ams_set_param out-band buffer size=%d\n", AMS_TEST_KB_NUM * 1024);
            printf("ams_test: ams_test_delay_mid=%u DSP_AMS_PARAM_ID_DELAY=0x%08x data=%p\n", ams_test_delay_mid, DSP_AMS_PARAM_ID_DELAY, data);
            // for (int i=0; i< 1000; i++){
            //     delay[i] = i;
            // }
            // for (int i=0; i< 1000; i++){
            //     if (delay[i] != i)
            //         printf("Error reading from shmem %d,%d",delay[i],i);
            // }
            rc = ams_set_param(ams_session, *pgh, ams_test_delay_mid, DSP_AMS_PARAM_ID_DELAY, AMS_TEST_KB_NUM * 1024, data);
            printf("ams_test: ams_set_param out-band result %d\n", rc);
        }
        else
        {
            rc = 1;
            printf("invalid args for set param, result %d NOK\n", rc);
        }
    }
#endif
#if 1
    {
        if (type == 1)
        {
            // get param test - inband
            dsp_ams_delay_param_t delay_param = {0};
            uint32_t param_size = sizeof(dsp_ams_delay_param_t);
            rc = ams_get_param(ams_session, *pgh, ams_test_delay_mid, DSP_AMS_PARAM_ID_DELAY,
                               &param_size, &delay_param);
            printf("ams_test: ams_get_param delay_param.delay_u %d\n", delay_param.delay_us);
        }
        else if (type == 2)
        {
            // get param test - outband
            uint32_t param_size = AMS_TEST_KB_NUM * 1024;
            memset(data, 0, AMS_TEST_KB_NUM * 1024);
            rc = ams_get_param(ams_session, *pgh, ams_test_delay_mid, DSP_AMS_PARAM_ID_DELAY,
                               &param_size, data);
            uint32_t *pdata = (uint32_t *)data;
            printf("ams_test: OOB ams_get_param delay_param.delay_u %d\n", *pdata);
        }
        else
        {
            rc = 1;
            printf("invalid args for get param, result %d NOK\n", rc);
        }
    }
#endif
    return rc;
}

static int ams_multilane_tdm_8155_sink(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh)
{
    ams_status_t rc = 0;
    ams_endpoint_t e1, e2;
    ams_module_t m1;
    ams_connection_t c1, c2;

    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    memset(&m1, 0, sizeof(m1));
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));

    rc = ams_create_graph(ams_session, param, pgh);
    e1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e1.type = AMS_ENDPOINT_TYPE_SINK;
    e1.channel_mask = 0xFF;
    e1.q_factor = 31;
    e1.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e1.channel_type, 0, sizeof(e1.channel_type));

    e1.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    e1.tdm_params.num_channels = 32;
    e1.tdm_params.bit_width = 32;
    e1.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e1.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e1.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e1.tdm_params.nslots_per_frame = 16;
    e1.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e1.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e1.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e1.tdm_params.slot_width = 32;
    e1.tdm_params.slot_mask = 0xFFFF;
    for (int i = 0; i < e1.tdm_params.num_channels; i++)
    {
        e1.channel_type[i] = i;
    }

    AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "%s : Multi-lane cfg for hw_intf : %d", __func__, e1.tdm_params.hw_interface_id);

    e2.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e2.type = AMS_ENDPOINT_TYPE_SOURCE;
    e2.channel_mask = 0xFF;
    e2.q_factor = 31;
    e2.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT;
    memset(e2.channel_type, 0, sizeof(e2.channel_type));

    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
    e2.tdm_params.num_channels = 8;
    e2.tdm_params.bit_width = 32;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = 8;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = 0xFF;
    for (int i = 0; i < e2.tdm_params.num_channels; i++)
    {
        e2.channel_type[i] = i;
    }

    m1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    ams_test_delay_mid = m1.id;

    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = 0x00013375;
    strlcpy(m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

    c1.num_channels = 8;
    c1.bit_width = 32;
    c1.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.source.endpoint.id = e2.id;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.destination.module.id = m1.id;
    c1.destination.module.port_index = 0;

    c2.num_channels = 8;
    c2.bit_width = 32;
    c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c2.source.module.id = m1.id;
    c2.source.module.port_index = 0;
    c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c2.destination.endpoint.id = e1.id;

    rc = ams_graph_add_endpont(ams_session, *pgh, &e1);

    rc |= ams_graph_add_endpont(ams_session, *pgh, &e2);

    rc |= ams_graph_add_module(ams_session, *pgh, &m1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c2);

    rc |= ams_util_print_graph_info(ams_session, *pgh);

    ams_graph_property_t prop;
    prop.prop_id = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG;
    prop.appy_after_start = 0;
    prop.u.exclv_ep_tdm_lane_cfg.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    prop.u.exclv_ep_tdm_lane_cfg.type = DSP_AMS_ENDPOINT_TYPE_SINK;
    prop.u.exclv_ep_tdm_lane_cfg.lane_mask = 10;

    rc = ams_graph_set_property(ams_session, *pgh, &prop);
    return rc;
}

static int ams_multilane_tdm_8155_source(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh)
{
    ams_status_t rc = 0;
    ams_endpoint_t e1, e2;
    ams_module_t m1;
    ams_connection_t c1, c2;

    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    memset(&m1, 0, sizeof(m1));
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));

    rc = ams_create_graph(ams_session, param, pgh);
    e1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e1.type = AMS_ENDPOINT_TYPE_SINK;
    e1.channel_mask = 0xFF;
    e1.q_factor = 31;
    e1.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e1.channel_type, 0, sizeof(e1.channel_type));

    e1.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM3;
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
    for (int i = 0; i < e1.tdm_params.num_channels; i++)
    {
        e1.channel_type[i] = i;
    }

    e2.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e2.type = AMS_ENDPOINT_TYPE_SOURCE;
    e2.channel_mask = 0xFF;
    e2.q_factor = 31;
    e2.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e2.channel_type, 0, sizeof(e2.channel_type));

    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    e2.tdm_params.num_channels = 32;
    e2.tdm_params.bit_width = 32;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = 16;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = 0xFFFF;
    for (int i = 0; i < e2.tdm_params.num_channels; i++)
    {
        e2.channel_type[i] = i;
    }

    AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "%s : Multi-lane cfg for hw_intf : %d", __func__, e2.tdm_params.hw_interface_id);

    m1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    ams_test_delay_mid = m1.id;

    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = 0x00013375;
    strlcpy(m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

    c1.num_channels = 8;
    c1.bit_width = 32;
    c1.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.source.endpoint.id = e2.id;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.destination.module.id = m1.id;
    c1.destination.module.port_index = 0;

    c2.num_channels = 8;
    c2.bit_width = 32;
    c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c2.source.module.id = m1.id;
    c2.source.module.port_index = 0;
    c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c2.destination.endpoint.id = e1.id;

    rc = ams_graph_add_endpont(ams_session, *pgh, &e1);

    rc |= ams_graph_add_endpont(ams_session, *pgh, &e2);

    rc |= ams_graph_add_module(ams_session, *pgh, &m1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c2);

    rc |= ams_util_print_graph_info(ams_session, *pgh);

    ams_graph_property_t prop;
    prop.prop_id = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG;
    prop.appy_after_start = 0;
    prop.u.exclv_ep_tdm_lane_cfg.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    prop.u.exclv_ep_tdm_lane_cfg.type = DSP_AMS_ENDPOINT_TYPE_SOURCE;
    prop.u.exclv_ep_tdm_lane_cfg.lane_mask = 5;

    rc = ams_graph_set_property(ams_session, *pgh, &prop);
    return rc;
}

static int ams_multilane_tdm_8155_concurrency(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh)
{
    ams_status_t rc = 0;
    ams_endpoint_t e1, e2;
    ams_module_t m1;
    ams_connection_t c1, c2;

    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    memset(&m1, 0, sizeof(m1));
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));

    rc = ams_create_graph(ams_session, param, pgh);
    e1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e1.type = AMS_ENDPOINT_TYPE_SINK;
    e1.channel_mask = 0xFFFF;
    e1.q_factor = 31;
    e1.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e1.channel_type, 0, sizeof(e1.channel_type));

    e1.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    e1.tdm_params.num_channels = 32;
    e1.tdm_params.bit_width = 32;
    e1.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e1.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e1.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e1.tdm_params.nslots_per_frame = 16;
    e1.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e1.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e1.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e1.tdm_params.slot_width = 32;
    e1.tdm_params.slot_mask = 0xFFFF;
    for (int i = 0; i < e1.tdm_params.num_channels; i++)
    {
        e1.channel_type[i] = i;
    }

    e2.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e2.type = AMS_ENDPOINT_TYPE_SOURCE;
    e2.channel_mask = 0xFF;
    e2.q_factor = 31;
    e2.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e2.channel_type, 0, sizeof(e2.channel_type));

    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    e2.tdm_params.num_channels = 8;
    e2.tdm_params.bit_width = 32;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = 8;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = 0xF;
    for (int i = 0; i < e2.tdm_params.num_channels; i++)
    {
        e2.channel_type[i] = i;
    }

    AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "%s : Concurrent Multi-lane cfg for hw_intf : %d as sink & hw_intf : %d as source",
                       __func__, e1.tdm_params.hw_interface_id, e2.tdm_params.hw_interface_id);

    m1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    ams_test_delay_mid = m1.id;

    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = 0x00013375;
    strlcpy(m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

    c1.num_channels = 8;
    c1.bit_width = 32;
    c1.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.source.endpoint.id = e2.id;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.destination.module.id = m1.id;
    c1.destination.module.port_index = 0;

    c2.num_channels = 8;
    c2.bit_width = 32;
    c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c2.source.module.id = m1.id;
    c2.source.module.port_index = 0;
    c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c2.destination.endpoint.id = e1.id;

    rc = ams_graph_add_endpont(ams_session, *pgh, &e1);

    rc |= ams_graph_add_endpont(ams_session, *pgh, &e2);

    rc |= ams_graph_add_module(ams_session, *pgh, &m1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c2);

    rc |= ams_util_print_graph_info(ams_session, *pgh);

    ams_graph_property_t prop;
    prop.prop_id = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG;
    prop.appy_after_start = 0;
    prop.u.exclv_ep_tdm_lane_cfg.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    prop.u.exclv_ep_tdm_lane_cfg.type = DSP_AMS_ENDPOINT_TYPE_SOURCE;
    prop.u.exclv_ep_tdm_lane_cfg.lane_mask = 5;
    rc = ams_graph_set_property(ams_session, *pgh, &prop);

    prop.prop_id = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG;
    prop.appy_after_start = 0;
    prop.u.exclv_ep_tdm_lane_cfg.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    prop.u.exclv_ep_tdm_lane_cfg.type = DSP_AMS_ENDPOINT_TYPE_SINK;
    prop.u.exclv_ep_tdm_lane_cfg.lane_mask = 10;
    rc = ams_graph_set_property(ams_session, *pgh, &prop);
    return rc;
}

static int ams_multilane_tdm_6155_sink(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh)
{
    ams_status_t rc = 0;
    ams_endpoint_t e1, e2;
    ams_module_t m1;
    ams_connection_t c1, c2;

    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    memset(&m1, 0, sizeof(m1));
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));

    rc = ams_create_graph(ams_session, param, pgh);
    e1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e1.type = AMS_ENDPOINT_TYPE_SINK;
    e1.channel_mask = 0xFF;
    e1.q_factor = 31;
    e1.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e1.channel_type, 0, sizeof(e1.channel_type));

    e1.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM5;
    e1.tdm_params.num_channels = 32;
    e1.tdm_params.bit_width = 32;
    e1.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e1.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e1.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e1.tdm_params.nslots_per_frame = 16;
    e1.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e1.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e1.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e1.tdm_params.slot_width = 32;
    e1.tdm_params.slot_mask = 0xFFFF;
    for (int i = 0; i < e1.tdm_params.num_channels; i++)
    {
        e1.channel_type[i] = i;
    }

    e2.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e2.type = AMS_ENDPOINT_TYPE_SOURCE;
    e2.channel_mask = 0xFF;
    e2.q_factor = 31;
    e2.flags = AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT;
    memset(e2.channel_type, 0, sizeof(e2.channel_type));

    AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "%s : Multi-lane cfg for hw_intf : %d", __func__, e1.tdm_params.hw_interface_id);

    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM4;
    e2.tdm_params.num_channels = 32;
    e2.tdm_params.bit_width = 32;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = 16;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = 0xFFFF;
    for (int i = 0; i < e2.tdm_params.num_channels; i++)
    {
        e2.channel_type[i] = i;
    }

    m1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    ams_test_delay_mid = m1.id;

    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = 0x00013375;
    strlcpy(m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

    c1.num_channels = 8;
    c1.bit_width = 32;
    c1.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.source.endpoint.id = e2.id;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.destination.module.id = m1.id;
    c1.destination.module.port_index = 0;

    c2.num_channels = 8;
    c2.bit_width = 32;
    c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c2.source.module.id = m1.id;
    c2.source.module.port_index = 0;
    c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c2.destination.endpoint.id = e1.id;

    rc = ams_graph_add_endpont(ams_session, *pgh, &e1);

    rc |= ams_graph_add_endpont(ams_session, *pgh, &e2);

    rc |= ams_graph_add_module(ams_session, *pgh, &m1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c2);

    rc |= ams_util_print_graph_info(ams_session, *pgh);

    ams_graph_property_t prop;
    prop.prop_id = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG;
    prop.appy_after_start = 0;
    prop.u.exclv_ep_tdm_lane_cfg.hw_interface_id = AMS_HW_INTERFACE_TDM5;
    prop.u.exclv_ep_tdm_lane_cfg.type = DSP_AMS_ENDPOINT_TYPE_SINK;
    prop.u.exclv_ep_tdm_lane_cfg.lane_mask = 3;

    rc = ams_graph_set_property(ams_session, *pgh, &prop);
    return rc;
}

static int ams_multilane_tdm_6155_source(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh)
{
    ams_status_t rc = 0;
    ams_endpoint_t e1, e2;
    ams_module_t m1;
    ams_connection_t c1, c2;

    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    memset(&m1, 0, sizeof(m1));
    memset(&c1, 0, sizeof(c1));
    memset(&c2, 0, sizeof(c2));

    rc = ams_create_graph(ams_session, param, pgh);
    e1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e1.type = AMS_ENDPOINT_TYPE_SINK;
    e1.channel_mask = 0xFF;
    e1.q_factor = 31;
    e1.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e1.channel_type, 0, sizeof(e1.channel_type));

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
    for (int i = 0; i < e1.tdm_params.num_channels; i++)
    {
        e1.channel_type[i] = i;
    }

    e2.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    e2.type = AMS_ENDPOINT_TYPE_SOURCE;
    e2.channel_mask = 0xFF;
    e2.q_factor = 31;
    e2.flags = AMS_ENDPOINT_EXCLUSIVE;
    memset(e2.channel_type, 0, sizeof(e2.channel_type));

    e2.tdm_params.hw_interface_id = AMS_HW_INTERFACE_TDM5;
    e2.tdm_params.num_channels = 32;
    e2.tdm_params.bit_width = 32;
    e2.tdm_params.data_format = AMS_TDM_LINEAR_PCM_DATA;
    e2.tdm_params.sync_mode = AMS_TDM_LONG_SYNC_MODE;
    e2.tdm_params.sync_src = AMS_TDM_SYNC_SRC_INTERNAL;
    e2.tdm_params.nslots_per_frame = 16;
    e2.tdm_params.ctrl_data_out_enable = AMS_TDM_CTRL_DATA_OE_DISABLE;
    e2.tdm_params.ctrl_invert_sync_pulse = AMS_TDM_SYNC_NORMAL;
    e2.tdm_params.ctrl_sync_data_delay = AMS_TDM_DATA_DELAY_0_BCLK_CYCLE;
    e2.tdm_params.slot_width = 32;
    e2.tdm_params.slot_mask = 0xFFFF;
    for (int i = 0; i < e2.tdm_params.num_channels; i++)
    {
        e2.channel_type[i] = i;
    }

    AMS_TEST_MSG_PRINT(AmsTestMsgLevel_High, "%s : Multi-lane cfg for hw_intf : %d", __func__, e2.tdm_params.hw_interface_id);

    m1.id = GET_NXT_UNIQUE_ID(ams_test_gid);
    ams_test_delay_mid = m1.id;

    m1.flags = AMS_INLINE_PROCESSING_MODE;
    m1.capiv2_info.id = 0x00013375;
    strlcpy(m1.capiv2_info.tag, "", sizeof(m1.capiv2_info.tag));
    strlcpy(m1.capiv2_info.shared_obj_filename, "", sizeof(m1.capiv2_info.shared_obj_filename));

    c1.num_channels = 8;
    c1.bit_width = 32;
    c1.source.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c1.source.endpoint.id = e2.id;
    c1.destination.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c1.destination.module.id = m1.id;
    c1.destination.module.port_index = 0;

    c2.num_channels = 8;
    c2.bit_width = 32;
    c2.source.type = AMS_CONNECTION_ELEMENT_TYPE_MODULE;
    c2.source.module.id = m1.id;
    c2.source.module.port_index = 0;
    c2.destination.type = AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT;
    c2.destination.endpoint.id = e1.id;

    rc = ams_graph_add_endpont(ams_session, *pgh, &e1);

    rc |= ams_graph_add_endpont(ams_session, *pgh, &e2);

    rc |= ams_graph_add_module(ams_session, *pgh, &m1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c1);

    rc |= ams_graph_add_connection(ams_session, *pgh, &c2);

    rc |= ams_util_print_graph_info(ams_session, *pgh);

    ams_graph_property_t prop;
    prop.prop_id = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG;
    prop.appy_after_start = 0;
    prop.u.exclv_ep_tdm_lane_cfg.hw_interface_id = AMS_HW_INTERFACE_TDM5;
    prop.u.exclv_ep_tdm_lane_cfg.type = DSP_AMS_ENDPOINT_TYPE_SOURCE;
    prop.u.exclv_ep_tdm_lane_cfg.lane_mask = 3;

    rc = ams_graph_set_property(ams_session, *pgh, &prop);
    return rc;
}

void show_multilane_tdm_test_cfg_options(void)
{
    printf("1: Mutilane cfg for 8155/8195 as sink \n");
    printf("2: Mutilane cfg for 8155/8195 as source \n");
    printf("3: Mutilane cfg for 8155/8195 as source & sink both concurrently \n");
    printf("4: Mutilane cfg for 6155 as sink \n");
    printf("5: Mutilane cfg for 6155 as source \n");
    return;
}

static ams_graph_handle_t *multi_lane_tdm_usecase_get_graph_phdl(int uc)
{
    ams_graph_handle_t *pgh = NULL;

    if ((uc > AMS_MULTILANE_TEST_UC_MAX) || (uc < AMS_TEST_UC_MIN))
    {
        goto exit;
    }
    pgh = &g_mlt_handles[uc - 1];

exit:
    return pgh;
}

typedef int (*create_multilane_tdm_func_t)(ams_session_t ams_session, ams_graph_basic_params_t *param, ams_graph_handle_t *pgh);
static create_multilane_tdm_func_t create_multilane_tdm_arr[AMS_MULTILANE_TEST_UC_MAX] = {
    &ams_multilane_tdm_8155_sink,
    &ams_multilane_tdm_8155_source,
    &ams_multilane_tdm_8155_concurrency,
    &ams_multilane_tdm_6155_sink,
    &ams_multilane_tdm_6155_source};
#ifdef PORTME_pcm_logging_test_1
static int pcm_logging_test_1(ams_session_t ams_session, ams_graph_basic_params_t *param, int sin_tone_duration)
{
    int rc = 0;
    int ams_teardown_delay_sec = 108;
    ams_graph_handle_t *pgh = NULL;

    printf("PCM logging performance test case 1 (maximum 2ch(ADSP) + 48ch(MDSP))\n");
    if (30 == sin_tone_duration)
    {
        printf("playback sin-tone wav file (10 sec 0s + 30 sec sin wav + 10 sec 0s) on device 106: audio_chime -d 106 /tmp/101030.wav &\n");
        rc = system("audio_chime -d 106 /tmp/101030.wav &");
        if (rc != 0)
        {
            printf("failed to run command: audio_chime -d 106 /tmp/101030.wav, rc = %d\n", rc);
            return rc;
        }
        ams_teardown_delay_sec = 10 + 30 + 5;
    }
    else if (100 == sin_tone_duration)
    {
        printf("playback sin-tone wav file (10 sec 0s + 100 sec sin wav + 10 sec 0s) on device 106: audio_chime -d 106 /tmp/101100.wav &\n");
        rc = system("audio_chime -d 106 /tmp/101100.wav &");
        if (rc != 0)
        {
            printf("failed to run command: audio_chime -d 106 /tmp/101100.wav, rc = %d\n", rc);
            return rc;
        }
        ams_teardown_delay_sec = 10 + 100 + 5;
    }
    else
    {
        printf("sin_tone_duration = %d, but we only support 30/100 second sin-tone wav file", sin_tone_duration);
        return 1;
    }
    delay(2000);

    printf("setup graph 4 (ANC case 1, graph A)\n");
    ams_test_target_id = AMS_TEST_TARGET_8150;
    set_usecase(4);
    pgh = usecase_get_graph_phdl(ams_usecase);
    if (pgh)
    {
        rc = create_gr_arr[ams_usecase - 1](ams_session, param, pgh);
    }
    else
    {
        printf("Unknown usecase!\n");
        return 1;
    }

    if (rc != 0)
    {
        printf("failed to create graph, rc = %d\n", rc);
        return rc;
    }

    pgh = usecase_get_graph_phdl(ams_usecase);
    rc = ams_open_graph(ams_session, *pgh);
    printf("ams_test: open graph returned value 0x%x\n", rc);
    if (rc != 0)
    {
        printf("failed to open graph\n");
        return rc;
    }

    pgh = usecase_get_graph_phdl(ams_usecase);
    rc = ams_start_graph(ams_session, *pgh);
    printf("ams_test: start graph returned value 0x%x\n", rc);
    if (rc != 0)
    {
        printf("failed to start graph\n");
        return rc;
    }

    printf("wait 108 second till the 100 second sin-tone wavform is played completely\n");
    delay(ams_teardown_delay_sec * 1000);

    pgh = usecase_get_graph_phdl(ams_usecase);
    rc = ams_stop_graph(ams_session, *pgh);
    printf("ams_test: stop graph returned value 0x%x\n", rc);
    if (rc != 0)
    {
        printf("failed to stop graph\n");
        return rc;
    }

    pgh = usecase_get_graph_phdl(ams_usecase);
    rc = ams_close_graph(ams_session, *pgh);
    printf("ams_test: close graph returned value 0x%x\n", rc);
    if (rc != 0)
    {
        printf("failed to close graph\n");
        return rc;
    }

    return rc;
}
#else

static int pcm_logging_test_1(ams_session_t ams_session, ams_graph_basic_params_t *param, int sin_tone_duration)
{
    int rc = 0;
    return rc;
}
#endif
// static int pcm_logging_test_2()
// {
//     int rc = 0;

//     printf("PCM logging performance test case 2 (maximum 16ch(ADSP) + 48ch(MDSP))!\n");

//     return rc;
// }

extern void share_mic_test(void);

#define BUFFERSIZE 3
int main(int argc, char **argv)
{
    int cnt = 0;
    int c = 0;
    int i = 0;
    char buffer[BUFFERSIZE];
    ams_status_t rc = 0;
    g_param.flags = 0;
    g_param.block_size = 12;
    g_param.processor_id = 0;
    g_param.sample_rate = 48000;
    ams_graph_handle_t *pgh = NULL;
    printf("ams_test version 1.0\n");
    show_options();
    // PCM loggint test case if "ams_test -a" or "ams_test -b"; ams restart "ams_test -r"
    while ((c = getopt(argc, argv, "abrdp:")) != -1)

    {
        if (i >= AMS_OPERATE_DEV_NUM_MAX)
        {
            printf("Max device number %d reached. start from 0! failed", AMS_OPERATE_DEV_NUM_MAX);
            goto quit;
        }
        switch (c)
        {
        case 'a':
            rc = ams_init(&ams_session);
            printf("ams_test: init returned value : 0x%x\n", rc);
            rc = ams_register_callback(ams_session, ams_ssr_user_cb_func, (void *)g_handles);
            printf("ams_test: register cb returned value : 0x%x\n", rc);
            rc = pcm_logging_test_1(ams_session, &g_param, 30);
            printf("%s\n", (rc == 0 ? "OK" : "NOK"));
            goto quit;
            break;

        case 'b':
            rc = ams_init(&ams_session);
            printf("ams_test: init returned value : 0x%x\n", rc);
            rc = ams_register_callback(ams_session, ams_ssr_user_cb_func, (void *)g_handles);
            printf("ams_test: register cb returned value : 0x%x\n", rc);
            rc = pcm_logging_test_1(ams_session, &g_param, 100);
            printf("%s\n", (rc == 0 ? "OK" : "NOK"));
            goto quit;
            break;
        case 'd':
        {
            printf("Option not supported!\n");
            break;
        }
        case 'p':
        {
            printf("Option not supported!\n");
            break;
        }
        case 'r':
        {
            uint32_t session_type = 0;
            ams_graph_handle_t pgh[AMS_TEST_UC_MAX];
            uint32_t num_gh = 0;
            rc = ams_restart(&ams_session, &session_type);
            printf("AMS restart returned %d, session type %d\n", rc, session_type);
            if (rc == 0 && session_type == AMS_SESSION_RESTARTED)
            {
                rc = ams_query_graph_list(ams_session, pgh, 1, &num_gh);
                printf("AMS query graph list returned %d, num_gh = %d\n", rc, num_gh);
                if (rc == 0)
                {
                    ams_graph_info_t graph_info;
                    uint32_t i, j;
                    for (i = 0; i < num_gh; i++)
                    {
                        set_usecase(i + 1);
                        g_handles[i] = pgh[i];
                        memset(&graph_info, 0, sizeof(ams_graph_info_t));
                        ams_test_delay_mid = 3; // set mid for the delay set test(graph 1)
                        rc = ams_query_graph_info(ams_session, pgh[i], &graph_info);
                        printf("AMS query graph info ret %d\n", rc);
                        if (rc == 0)
                        {
                            printf("AMS graph modules num %d\n", graph_info.modules_num);
                            printf("AMS graph state %d\n", graph_info.state);
                            printf("AMS graph ep num %d\n", graph_info.ep_num);
                            for (j = 0; j < graph_info.modules_num; j++)
                            {
                                printf("AMS graph module id %d\n", graph_info.modules_ids[j]);
                            }
                            for (j = 0; j < graph_info.ep_num; j++)
                            {
                                printf("AMS graph ep id %d\n", graph_info.ep_ids[j]);
                            }
                        }
                    }
                }
            }
            if (rc == 0)
            {
                rc = ams_register_callback(ams_session, ams_ssr_user_cb_func, (void *)g_handles);
                printf("ams_test: register cb returned value : 0x%x\n", rc);
            }
        }
        break;
        default:
            break;
        }
    }

    show_options();
    while (1)
    {
        while (fgets(buffer, BUFFERSIZE, stdin))
        {
            cnt++;
            switch (buffer[0])
            {
            case '0':
                rc = show_options();
                printf("%s\n", (rc == 0 ? "OK" : "NOK"));
                break;
            case '1':
            {
                int uc_set = 0;
                while (1)
                {
                    printf("Enter usecase id (min=%d, max = %d):\n", AMS_TEST_UC_MIN, AMS_TEST_UC_MAX);
                    while (fgets(buffer, BUFFERSIZE, stdin))
                    {
                        int uc = buffer[0] - '0';
                        if (uc >= AMS_TEST_UC_MIN && uc <= AMS_TEST_UC_MAX)
                        {
                            set_usecase(uc);
                            printf("Usecase is %d\n", uc);
                            uc_set = 1;

                            if (uc == 1)
                            {
                                printf("Enter target: 2 - 8150/8195; 1 - 6150 :\n");
                                while (1)
                                {
                                    while (fgets(buffer, BUFFERSIZE, stdin))
                                    {
                                        ams_test_target_id = buffer[0] - '0';
                                        if ((ams_test_target_id >= AMS_TEST_TARGET_6150) && (ams_test_target_id <= AMS_TEST_TARGET_8150))
                                        {
                                            if (ams_test_target_id == 1)
                                            {
                                                printf("target is 6150\n");
                                            }
                                            else
                                            {
                                                printf("target is 8150/8195\n");
                                            }
                                            break;
                                        }
                                    }

                                    if (ams_test_target_id)
                                        break;
                                }
                            }
                            show_options();
                            break;
                        }
                    }
                    if (uc_set)
                        break;
                }
                break;
            }
            case '2':
            {
                uint32_t state = 0;
                if (ams_session == NULL)
                {
                    rc = ams_init(&ams_session);
                    printf("ams_test: init returned value : 0x%x\n", rc);
                    rc = ams_register_callback(ams_session, ams_ssr_user_cb_func, (void *)g_handles);
                    printf("ams_test: register cb returned value : 0x%x\n", rc);
                }
                pgh = usecase_get_graph_phdl(ams_usecase);
                if (pgh)
                {
                    rc = create_gr_arr[ams_usecase - 1](ams_session, &g_param, pgh);
                }
                else
                {
                    printf("Unknown usecase!\n");
                    goto quit;
                }
                printf("Create graph %d %s\n", ams_usecase, (rc == 0 ? "OK" : "NOK"));
                break;
            }
            case '3':
                pgh = usecase_get_graph_phdl(ams_usecase);
                if (pgh)
                {
                    rc = open_graph(ams_session, *pgh);
                    printf("Open graph(0x%lx) %d %s\n", (uintptr_t)*pgh, ams_usecase, (rc == 0 ? "OK" : "NOK"));
                }
                break;
            case '4':
                pgh = usecase_get_graph_phdl(ams_usecase);
                if (pgh)
                {
                    rc = start_graph(ams_session, *pgh);
                    printf("Start graph(0x%lx) %d %s\n", (uintptr_t)*pgh, ams_usecase, (rc == 0 ? "OK" : "NOK"));
                }
                break;
            case '5':
            {
                int set_param = 0;
                printf("Enter type (in-band = 1, out-band = 2):\n");
                while (1)
                {
                    while (fgets(buffer, BUFFERSIZE, stdin))
                    {
                        int type = buffer[0] - '0';
                        if (type == 1 || type == 2)
                        {
                            rc = set_delay_param(type);
                            printf("Set delay param of graph 1 %s\n", (rc == 0 ? "OK" : "NOK"));
                            set_param = 1;
                            break;
                        }
                        else
                            break;
                    }
                    if (set_param)
                        break;
                }
                break;
            }
            case '6':
                rc = start_playback();
                printf("Start playback %s\n", (rc == 0 ? "OK" : "NOK"));
                break;
            case '7':
                pgh = usecase_get_graph_phdl(ams_usecase);
                if (pgh)
                {
                    rc = stop_graph(ams_session, *pgh);
                    printf("Stop graph %d %s\n", ams_usecase, (rc == 0 ? "OK" : "NOK"));
                }
                else
                {
                    printf("Invalid Usecase\n");
                }
                break;
            case '8':
                pgh = usecase_get_graph_phdl(ams_usecase);
                if (pgh)
                {
                    rc = close_graph(ams_session, *pgh);
                    printf("Close graph %d %s\n", ams_usecase, (rc == 0 ? "OK" : "NOK"));
                }
                else
                {
                    printf("Invalid Usecase\n");
                }
                break;
            case '9':
                pgh = usecase_get_graph_phdl(ams_usecase);
                if (pgh)
                {
                    rc = destroy_graph(ams_session, pgh);
                    printf("Destroy graph %d %s\n", ams_usecase, (rc == 0 ? "OK" : "NOK"));
                }
                else
                {
                    printf("Invalid Usecase\n");
                }
                break;
            case 's':
                // call ssr
                printf("Not supported\n");
                break;
            case 'q':
                printf("Quit\n");
                goto quit;
            case 'a':
                share_mic_test();
                printf("end of share_mic_test\n");
                show_options();
                break;
            case 'm':
                show_multilane_tdm_test_cfg_options();
                printf("Enter multi-lane tdm usecase id (min=%d, max=%d):\n", AMS_TEST_UC_MIN, AMS_MULTILANE_TEST_UC_MAX);
                fgets(buffer, BUFFERSIZE, stdin);
                int uc = buffer[0] - '0';
                if (uc > AMS_MULTILANE_TEST_UC_MAX || uc < AMS_TEST_UC_MIN)
                {
                    printf("Invalid Usecase :%d", uc);
                    goto quit;
                }
                set_usecase(uc);
                pgh = multi_lane_tdm_usecase_get_graph_phdl(ams_usecase);
                if (!pgh)
                {
                    printf("Unknown usecase : %d!\n", ams_usecase);
                    goto quit;
                }
                rc = create_multilane_tdm_arr[ams_usecase - 1](ams_session, &g_param, pgh);
                printf("Set Multilane usecase (0x%lx) %d %s\n", (uintptr_t)*pgh, ams_usecase, (rc == 0 ? "OK" : "NOK"));
                show_options();
                break;
            default:
                break;
            }
        }
    }
quit:
    rc = close_app();
    printf("Close app %s\n", (rc == 0 ? "OK" : "NOK"));
#ifdef AMS_CORE_TEST

#endif
    exit(0);
}
