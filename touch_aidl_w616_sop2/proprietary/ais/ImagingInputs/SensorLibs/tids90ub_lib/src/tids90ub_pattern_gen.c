/**
 * @file tids90ub_pattern_gen.c
 *
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "tids90ub_pattern_gen.h"

#define INIT_DESER \
{\
    { 0x58, 0x5e, _tids90ub_delay_ }, \
}

#define START_pattern_gen  \
{\
    { 0x32, 0x01, _tids90ub_delay_ }, \
    { 0x33, 0x03, _tids90ub_delay_ }, \
}

#define INIT_pattern_gen {}
#define STOP_pattern_gen {}

static struct camera_i2c_reg_array tids90ub_pattern_gen_tids90ub_init[] = INIT_DESER;
static struct camera_i2c_reg_array tids90ub_start_pattern_gen[] = START_pattern_gen;
static struct camera_i2c_reg_array tids90ub_start_reg_array_tpg_2MP[]= START_REG_ARRAY_TPG_2MP;
static struct camera_i2c_reg_array tids90ub_start_reg_array_tpg_4MP[]= START_REG_ARRAY_TPG_4MP;
static struct camera_i2c_reg_array tids90ub_start_reg_array_tpg_8MP[]= START_REG_ARRAY_TPG_8MP;
static struct camera_i2c_reg_array tids90ub_start_reg_array_tpg_rgb_888_fhd_color_bar[]= START_REG_ARRAY_TPG_RGB_888_FHD_COLOR_BAR;
static struct camera_i2c_reg_array tids90ub_start_reg_array_tpg_rgb_888_hd_color_bar[]= START_REG_ARRAY_TPG_RGB_888_HD_COLOR_BAR;
static struct camera_i2c_reg_array tids90ub_start_reg_array_tpg_rgb_888_fhd_fixed_color[]= START_REG_ARRAY_TPG_RGB_888_FHD_FIXED_COLOR;
static struct camera_i2c_reg_array tids90ub_start_reg_array_tpg_rgb_888_hd_fixed_color[]= START_REG_ARRAY_TPG_RGB_888_HD_FIXED_COLOR;

static img_src_mode_t tids90ub_pattern_gen_output_mode[TIDS90UB_TPG_MODE_MAX] =
{
    [TIDS90UB_TPG_MODE_2MP] =
    {
        .fmt = TPG_SENSOR_FORMAT,
        .res = {.width = TPG_SENSOR_WIDTH_2MP, .height = TPG_SENSOR_HEIGHT_2MP, .fps = 30.0f},
        .channel_info = {.vc = 0, .dt = TPG_SENSOR_DT, .cid = 0,},
    },
    [TIDS90UB_TPG_MODE_4MP] =
    {
        .fmt = TPG_SENSOR_FORMAT,
        .res = {.width = TPG_SENSOR_WIDTH_4MP, .height = TPG_SENSOR_HEIGHT_4MP, .fps = 30.0f},
        .channel_info = {.vc = 0, .dt = TPG_SENSOR_DT, .cid = 0,},
    },
    [TIDS90UB_TPG_MODE_8MP] =
    {
        .fmt = TPG_SENSOR_FORMAT,
        .res = {.width = TPG_SENSOR_WIDTH_8MP, .height = TPG_SENSOR_HEIGHT_8MP, .fps = 30.0f},
        .channel_info = {.vc = 0, .dt = TPG_SENSOR_DT, .cid = 0,},
    },
    [TIDS90UB_TPG_MODE_RGB_888_FHD_COLOR_BAR] =
    {
        .fmt = TPG_SENSOR_FORMAT_RGB_888,
        .res = {.width = TPG_SENSOR_WIDTH_RGB_888_FHD, .height = TPG_SENSOR_HEIGHT_RGB_888_FHD, .fps = 30.0f},
        .channel_info = {.vc = 0, .dt = TPG_SENSOR_DT_RGB_888, .cid = 0,},
    },
    [TIDS90UB_TPG_MODE_RGB_888_HD_COLOR_BAR] =
    {
        .fmt = TPG_SENSOR_FORMAT_RGB_888,
        .res = {.width = TPG_SENSOR_WIDTH_RGB_888_HD, .height = TPG_SENSOR_HEIGHT_RGB_888_HD, .fps = 30.0f},
        .channel_info = {.vc = 0, .dt = TPG_SENSOR_DT_RGB_888, .cid = 0,},
    },
    [TIDS90UB_TPG_MODE_RGB_888_FHD_FIXED_COLOR] =
    {
        .fmt = TPG_SENSOR_FORMAT_RGB_888,
        .res = {.width = TPG_SENSOR_WIDTH_RGB_888_FHD, .height = TPG_SENSOR_HEIGHT_RGB_888_FHD, .fps = 30.0f},
        .channel_info = {.vc = 0, .dt = TPG_SENSOR_DT_RGB_888, .cid = 0,},
    },
    [TIDS90UB_TPG_MODE_RGB_888_HD_FIXED_COLOR] =
    {
        .fmt = TPG_SENSOR_FORMAT_RGB_888,
        .res = {.width = TPG_SENSOR_WIDTH_RGB_888_HD, .height = TPG_SENSOR_HEIGHT_RGB_888_HD, .fps = 30.0f},
        .channel_info = {.vc = 0, .dt = TPG_SENSOR_DT_RGB_888, .cid = 0,},
    },
};

static int tids90ub_pattern_gen_detect(tids90ub_context_t* ctxt, uint32 port);
static int tids90ub_pattern_gen_get_port_cfg(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* p_cfg);
static int tids90ub_pattern_gen_init_port(tids90ub_context_t* ctxt, uint32 port);
static int tids90ub_pattern_gen_start_port(tids90ub_context_t* ctxt, uint32 port);
static int tids90ub_pattern_gen_stop_port(tids90ub_context_t* ctxt, uint32 port);
static int tids90ub_set_port_mode(tids90ub_context_t* ctxt, uint32 port, tids90ub_sensor_port_mode_t mode);
static int tids90ub_pattern_gen_apply_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info);
static int tids90ub_pattern_gen_apply_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure);

static tids90ub_sensor_t tids90ub_pattern_gen_info = {
    .detect = tids90ub_pattern_gen_detect,
    .get_port_cfg = tids90ub_pattern_gen_get_port_cfg,

    .init_port = tids90ub_pattern_gen_init_port,
    .start_port = tids90ub_pattern_gen_start_port,
    .stop_port = tids90ub_pattern_gen_stop_port,
    .set_port_mode = tids90ub_set_port_mode,
    .apply_exposure = tids90ub_pattern_gen_apply_exposure,
    .apply_hdr_exposure = tids90ub_pattern_gen_apply_hdr_exposure
};


static struct camera_i2c_reg_setting tids90ub_pattern_gen_reg_setting =
{
    .reg_array = NULL,
    .size = 0,
    .addr_type = CAMERA_I2C_BYTE_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
};

tids90ub_sensor_t* tids90ub_pattern_gen_get_sensor_info(void)
{
    return &tids90ub_pattern_gen_info;
}

static int tids90ub_pattern_gen_detect(tids90ub_context_t* ctxt, uint32 port)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};

    /*read chip id*/
    tids90ub_pattern_gen_reg_setting.reg_array = read_reg;
    tids90ub_pattern_gen_reg_setting.size = STD_ARRAY_SIZE(read_reg);

    tids90ub_pattern_gen_reg_setting.reg_array[0].delay = 0;
    tids90ub_pattern_gen_reg_setting.reg_array[0].reg_data = 0;
    tids90ub_pattern_gen_reg_setting.reg_array[0].reg_addr = 0;

    SLOW("Detected tids90ub test pattern generator Ser 0x%x", read_reg[0].reg_data);

    pSensor->state = TIDS90UB_SENSOR_STATE_DETECTED;
    switch(pCtxt->config.sensors[port].sensor_mode)
    {

        case TIDS90UB_TPG_MODE_2MP:
            pCtxt->tids90ub_reg_setting.reg_array = tids90ub_start_reg_array_tpg_2MP;
            pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_start_reg_array_tpg_2MP);
            break;
        case TIDS90UB_TPG_MODE_4MP:
            pCtxt->tids90ub_reg_setting.reg_array = tids90ub_start_reg_array_tpg_4MP;
            pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_start_reg_array_tpg_4MP);
            break;
        case TIDS90UB_TPG_MODE_8MP:
            pCtxt->tids90ub_reg_setting.reg_array = tids90ub_start_reg_array_tpg_8MP;
            pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_start_reg_array_tpg_8MP);
            break;
        case TIDS90UB_TPG_MODE_RGB_888_FHD_COLOR_BAR:
            pCtxt->tids90ub_reg_setting.reg_array = tids90ub_start_reg_array_tpg_rgb_888_fhd_color_bar;
            pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_start_reg_array_tpg_rgb_888_fhd_color_bar);
            break;
        case TIDS90UB_TPG_MODE_RGB_888_HD_COLOR_BAR:
            pCtxt->tids90ub_reg_setting.reg_array = tids90ub_start_reg_array_tpg_rgb_888_hd_color_bar;
            pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_start_reg_array_tpg_rgb_888_hd_color_bar);
            break;    
        case TIDS90UB_TPG_MODE_RGB_888_FHD_FIXED_COLOR:
            pCtxt->tids90ub_reg_setting.reg_array = tids90ub_start_reg_array_tpg_rgb_888_fhd_fixed_color;
            pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_start_reg_array_tpg_rgb_888_fhd_fixed_color);
            break;
        case TIDS90UB_TPG_MODE_RGB_888_HD_FIXED_COLOR:
            pCtxt->tids90ub_reg_setting.reg_array = tids90ub_start_reg_array_tpg_rgb_888_hd_fixed_color;
            pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_start_reg_array_tpg_rgb_888_hd_fixed_color);
            break;
        default:
            SERR("TPG Mode/resolution is not supported");
            rc = -1;
    }

    if(TIDS90UB_SENSOR_ID_SER_PATTERN_GEN == pCtxt->config.sensors[port].sensor_id)
    {
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &pCtxt->tids90ub_reg_setting);
        if (rc)
        {
            SERR("ti953 test pattern generator 0x%x failed to start", pSensor->serializer_alias);
        }
    }
    else
    {
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pCtxt->slave_addr,
            &pCtxt->tids90ub_reg_setting);
        if (rc)
        {
            SERR("tids90ub test pattern generator 0x%x failed to start", pSensor->serializer_alias);
        }
    }

    tids90ub_pattern_gen_reg_setting.reg_array = NULL;

    return rc;
}

static int tids90ub_pattern_gen_get_port_cfg(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* p_cfg)
{
    (void)ctxt;

    p_cfg->sources[0] = tids90ub_pattern_gen_output_mode[ctxt->config.sensors[port].sensor_mode];
    p_cfg->sources[0].channel_info.vc = port;
    p_cfg->num_sources = 1;

    p_cfg->deser_config = tids90ub_pattern_gen_tids90ub_init;
    p_cfg->deser_config_size = STD_ARRAY_SIZE(tids90ub_pattern_gen_tids90ub_init);

    return 0;
}

static int tids90ub_pattern_gen_init_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_DETECTED == pSensor->state)
    {
        pSensor->state = TIDS90UB_SENSOR_STATE_INITIALIZED;
        SLOW("tids90ub test pattern generator init state %d", pSensor->state);
    }
    else
    {
        SERR("tids90ub test pattern generator %d init in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int tids90ub_pattern_gen_start_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SHIGH("starting tids90ub test pattern generator ");

        if(TIDS90UB_SENSOR_ID_PATTERN_GEN == pCtxt->config.sensors[port].sensor_id)
        {
            pCtxt->tids90ub_reg_setting.reg_array = tids90ub_start_pattern_gen;
            pCtxt->tids90ub_reg_setting.size = STD_ARRAY_SIZE(tids90ub_start_pattern_gen);

            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                pCtxt->ctrl,
                pCtxt->slave_addr,
                &pCtxt->tids90ub_reg_setting);
            
            if (rc)
            {
                SERR("tids90ub test pattern generator 0x%x failed to start", pSensor->serializer_alias);
            }
        }
    }
    else
    {
        SERR("tids90ub test pattern generator %d start in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int tids90ub_pattern_gen_stop_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_STREAMING == pSensor->state)
    {

        SLOW("tids90ub test pattern generator stop (0x%x)", pSensor->serializer_alias);
    }
    else
    {
        SERR("tids90ub test pattern generator %d stop in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int tids90ub_set_port_mode(tids90ub_context_t* ctxt, uint32 port, tids90ub_sensor_port_mode_t mode)
{
    int rc = 0;

    CAM_UNUSED(ctxt);
    CAM_UNUSED(port);
    CAM_UNUSED(mode);

    return rc;
}

static int tids90ub_pattern_gen_apply_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info)
{
    int rc = 0;

    CAM_UNUSED(ctxt);
    CAM_UNUSED(port);
    CAM_UNUSED(exposure_info);

    return rc;
}

static int tids90ub_pattern_gen_apply_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    int rc = 0;

    CAM_UNUSED(ctxt);
    CAM_UNUSED(port);
    CAM_UNUSED(hdr_exposure);

    return rc;
}

