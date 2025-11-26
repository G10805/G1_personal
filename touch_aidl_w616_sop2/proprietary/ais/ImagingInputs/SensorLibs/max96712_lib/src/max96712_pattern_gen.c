/**
 * @file max96712_pattern_gen.c
 *
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "max96712_pattern_gen.h"

static struct camera_i2c_reg_array max96712_start_tpg_2MP_2ln[] = START_REG_ARRAY_TPG_2MP_2LN;
static struct camera_i2c_reg_array max96712_start_reg_array_tpg_2MP_10[]= START_REG_ARRAY_TPG_2MP_10;
static struct camera_i2c_reg_array max96712_start_reg_array_tpg_2MP_30[]= START_REG_ARRAY_TPG_2MP_30;
static struct camera_i2c_reg_array max96712_start_reg_array_tpg_8MP[]= START_REG_ARRAY_TPG_8MP;

static maxim_pipeline_t max96712_pattern_gen_isp_pipelines[MAXIM_TPG_MODE_MAX] =
{
    [MAXIM_TPG_MODE_2MP_10] =
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = SENSOR_FORMAT,
            .res = {.width = SENSOR_WIDTH_2MP_10, .height = SENSOR_HEIGHT_2MP_10, .fps = 10.0f},
            .channel_info = {.vc = 1, .dt = SENSOR_DT, .cid = 0,},
        }
    },
    [MAXIM_TPG_MODE_2MP_30] =
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = SENSOR_FORMAT,
            .res = {.width = SENSOR_WIDTH_2MP_30, .height = SENSOR_HEIGHT_2MP_30, .fps = 30.0f},
            .channel_info = {.vc = 1, .dt = SENSOR_DT, .cid = 0,},
        }
    },
    [MAXIM_TPG_MODE_8MP] =
    {
        .id = MAXIM_PIPELINE_X,
        .mode =
        {
            .fmt = SENSOR_FORMAT,
            .res = {.width = SENSOR_WIDTH_8MP, .height = SENSOR_HEIGHT_8MP, .fps = 30.0f},
            .channel_info = {.vc = 1, .dt = SENSOR_DT, .cid = 0,},
        }
    },
};

static int max96712_pattern_gen_detect(max96712_context_t* ctxt, uint32 link);
static int max96712_pattern_gen_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg);
static int max96712_pattern_gen_init_link(max96712_context_t* ctxt, uint32 link);
static int max96712_pattern_gen_start_link(max96712_context_t* ctxt, uint32 link);
static int max96712_pattern_gen_stop_link(max96712_context_t* ctxt, uint32 link);

max96712_sensor_t max96712_pattern_gen_info = {
    .id = MAXIM_SENSOR_ID_PATTERN_GEN,
    .detect = max96712_pattern_gen_detect,
    .get_link_cfg = max96712_pattern_gen_get_link_cfg,

    .init_link = max96712_pattern_gen_init_link,
    .start_link = max96712_pattern_gen_start_link,
    .stop_link = max96712_pattern_gen_stop_link,
};

static struct camera_i2c_reg_setting max96712_pattern_gen_reg_setting =
{
    .reg_array = NULL,
    .size = 0,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
};

max96712_sensor_t* max96712_pattern_gen_get_sensor_info(void)
{
    return &max96712_pattern_gen_info;
}

static int max96712_pattern_gen_detect(max96712_context_t* ctxt, uint32 link)
{
    int rc = 0;
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    
    pSensor->state = SENSOR_STATE_DETECTED;
    SHIGH("Detected max96712 test pattern generator");
    return rc;
}

static int max96712_pattern_gen_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg)
{
    (void)ctxt;
    unsigned int mode;
    if (ctxt->max96712_sensors[link].mode < MAXIM_TPG_MODE_MAX)
    {
        mode = ctxt->max96712_sensors[link].mode;
    }
    else
    {
        SERR("The mode set is greater than tha MAX mode supported. Setting mode to Default.");
        mode = MAXIM_TPG_MODE_DEFAULT;
    }
    p_cfg->num_pipelines = 1;
    p_cfg->pipelines[0] = max96712_pattern_gen_isp_pipelines[mode];

    return 0;
}

static int max96712_pattern_gen_init_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    int rc = 0;
    
    SHIGH("max96712_pattern_gen_init_link()");    
    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        pSensor->state = SENSOR_STATE_INITIALIZED;
        SHIGH("max96712 test pattern generator init state %d", pSensor->state);
    }
    else
    {
        SERR("max96712 test pattern generator %d init in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int max96712_pattern_gen_start_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    int rc = 0;

    SHIGH("max96712_pattern_gen_start_link()");

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SHIGH("starting max96712 test pattern generator");

        switch(pCtxt->max96712_config.sensors[link].mode)
        {
        case MAXIM_TPG_MODE_2MP_10:
            max96712_pattern_gen_reg_setting.reg_array = max96712_start_reg_array_tpg_2MP_10;
            max96712_pattern_gen_reg_setting.size = STD_ARRAY_SIZE(max96712_start_reg_array_tpg_2MP_10);
            break;
        case MAXIM_TPG_MODE_2MP_30:
            max96712_pattern_gen_reg_setting.reg_array = max96712_start_reg_array_tpg_2MP_30;
            max96712_pattern_gen_reg_setting.size = STD_ARRAY_SIZE(max96712_start_reg_array_tpg_2MP_30);
            break;
        case MAXIM_TPG_MODE_8MP:
            max96712_pattern_gen_reg_setting.reg_array = max96712_start_reg_array_tpg_8MP;
            max96712_pattern_gen_reg_setting.size = STD_ARRAY_SIZE(max96712_start_reg_array_tpg_8MP);
            break;
        default:
            SERR("TPG Mode/resolution is not supported");
            return -1;
            break;
        }

        if(MAXIM_SENSOR_ID_SER_PATTERN_GEN == pCtxt->max96712_config.sensors[link].id)
        {
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pSensor->serializer_alias,
                &max96712_pattern_gen_reg_setting);
            if (rc)
            {
                SERR("SER test pattern generator 0x%x failed to start", pSensor->serializer_alias);
            }
        }
        else
        {
            //override for 2 lanes
            if (pCtxt->max96712_config.opMode == MAXIM_OP_MODE_2_LANES)
            {
                max96712_pattern_gen_reg_setting.reg_array = max96712_start_tpg_2MP_2ln;
                max96712_pattern_gen_reg_setting.size = STD_ARRAY_SIZE(max96712_start_tpg_2MP_2ln);
            }

            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &max96712_pattern_gen_reg_setting);
            if (rc)
            {
                SERR("max96712 test pattern generator 0x%x failed to start", pCtxt->slave_addr);
            }
        }
    }
    else
    {
        SERR("max96712 test pattern generator %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int max96712_pattern_gen_stop_link(max96712_context_t* ctxt, uint32 link)
{    
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    int rc = 0;
    
    SHIGH("max96712_pattern_gen_stop_link()");    
    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        SLOW("max96712 test pattern generator stop (0x%x)", pSensor->serializer_alias);
    }
    else
    {
        SERR("max96712 test pattern generator %d stop in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}
