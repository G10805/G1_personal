/**
 * @file imx_gw5200.c
 *
 * Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>
#include "imx_gw5200.h"
#include "vendor_ext_properties.h"

#define INIT_DESER \
{ \
    { 0x5B, 0x30, _tids90ub_delay_}, \
    { 0x5d, SENSOR_DEFAULT_ADDR, _tids90ub_delay_ }, \
}

#define SYNCMODE_imx_gw5200 \
{ \
    { 0x03, 0x50, _imx_gw5200_delay_}, \
}

#define BACKCOMP_MODE_imx_gw5200 \
{ \
    { 0xB0, 0x04, _imx_gw5200_delay_}, \
    { 0xB1, 0x48, _imx_gw5200_delay_}, \
    { 0xB2, 0x29, _imx_gw5200_delay_}, \
    { 0x01, 0x01, _imx_gw5200_delay_}, \
}

#define BC_GPIO0_EN \
{ \
    { 0x0E, 0x10, _imx_gw5200_delay_}, \
    { 0x0D, 0x10, _imx_gw5200_delay_}, \
}

#define I2C_BULKMODE_TESTING_ARR_API_CMD \
    {0x33, 0x47, 0x03, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x0D}

#define I2C_BULKMODE_TESTING_ARR_QUERY_CMD \
    {0x33, 0x51, 0x07, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00}

#define I2C_BULKMODE_TESTING_CMD_MSGID_IDX 3

#define INIT_imx_gw5200 {}
#define START_imx_gw5200 {}
#define STOP_imx_gw5200 {}
#define ENABLE_BULK_WRITEREAD_TESTING 1

static struct camera_i2c_reg_array imx_gw5200_tids90ub_init[] = INIT_DESER;

static struct camera_i2c_reg_array imx_gw5200_mode_sel[] = SYNCMODE_imx_gw5200;
static struct camera_i2c_reg_array imx_gw5200_backw_comp_sel[] = BACKCOMP_MODE_imx_gw5200;
static struct camera_i2c_reg_array imx_gw5200_set_fsync[] = BC_GPIO0_EN;

#if 0 //@todo
static struct camera_i2c_reg_array imx_gw5200_init[] = INIT_imx_gw5200;
static struct camera_i2c_reg_array imx_gw5200_start[] = START_imx_gw5200;
static struct camera_i2c_reg_array imx_gw5200_stop[] = STOP_imx_gw5200;
#endif

static img_src_channel_info_t imx_gw5200_channel_info[TIDS90UB_PORT_MAX] =
{
    {.vc = VC0, .dt = SENSOR_DT, .cid = CID_VC0},
    {.vc = VC1, .dt = SENSOR_DT, .cid = CID_VC1},
    {.vc = VC2, .dt = SENSOR_DT, .cid = CID_VC2},
    {.vc = VC3, .dt = SENSOR_DT, .cid = CID_VC3}
};

static img_src_mode_t imx424_gw5200_ti953_output_mode[IMX424_GW5200_TI953_MODE_MAX] =
{
  [IMX424_GW5200_TI953_MODE_2MP] =
  {
      .fmt = SENSOR_FORMAT,
      .res = {.width = IMX424_2MP_SENSOR_WIDTH, .height = IMX424_2MP_SENSOR_HEIGHT, .fps = 30.0f},
  },
  [IMX424_GW5200_TI953_MODE_7_4MP] =
  {
      .fmt = SENSOR_FORMAT,
      .res = {.width = IMX424_7_4MP_SENSOR_WIDTH, .height = IMX424_7_4MP_SENSOR_HEIGHT, .fps = 30.0f},
  },
};

static img_src_mode_t imx490_gw5200_ti953_output_mode =
{
  .fmt = SENSOR_FORMAT,
  .res = {.width = IMX490_5MP_SENSOR_WIDTH, .height = IMX490_5MP_SENSOR_HEIGHT, .fps = 30.0f},
};

static img_src_mode_t imx424_gw5200_ti951_output_mode[IMX424_GW5200_TI951_MODE_MAX] =
{
    [IMX424_GW5200_TI951_MODE_4MP] =
    {
        .fmt = SENSOR_FORMAT,
        .res = {.width = IMX424_4MP_SENSOR_WIDTH, .height = IMX424_4MP_SENSOR_HEIGHT, .fps = 30.0f},
    },
    [IMX424_GW5200_TI951_MODE_1_7MP] =
    {
        .fmt = SENSOR_FORMAT,
        .res = {.width = IMX424_1_7MP_SENSOR_WIDTH, .height = IMX424_1_7MP_SENSOR_HEIGHT, .fps = 30.0f},
    },
    [IMX424_GW5200_TI951_MODE_7_4MP] =
    {
        .fmt = SENSOR_FORMAT,
        .res = {.width = IMX424_7_4MP_SENSOR_WIDTH, .height = IMX424_7_4MP_SENSOR_HEIGHT, .fps = 30.0f},
    },
};

static img_src_mode_t imx424_gw5200_ti971_output_mode[IMX424_GW5200_TI971_MODE_MAX] =
{
    [IMX424_GW5200_TI971_MODE_1_7MP] =
    {
        .fmt = SENSOR_FORMAT,
        .res = {.width = IMX424_1_7MP_SENSOR_WIDTH, .height = IMX424_1_7MP_SENSOR_HEIGHT, .fps = 30.0f},
    },
    [IMX424_GW5200_TI971_MODE_4MP] =
    {
        .fmt = SENSOR_FORMAT,
        .res = {.width = IMX424_4MP_SENSOR_WIDTH, .height = IMX424_4MP_SENSOR_HEIGHT, .fps = 30.0f},
    },
    [IMX424_GW5200_TI971_MODE_7_4MP] =
    {
        .fmt = SENSOR_FORMAT,
        .res = {.width = IMX424_7_4MP_SENSOR_WIDTH, .height = IMX424_7_4MP_SENSOR_HEIGHT, .fps = 30.0f},
    },
};

static int imx_gw5200_detect(tids90ub_context_t* ctxt, uint32 port);
static int imx_gw5200_get_port_cfg(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* p_cfg);
static int imx_gw5200_init_port(tids90ub_context_t* ctxt, uint32 port);
static int imx_gw5200_start_port(tids90ub_context_t* ctxt, uint32 port);
static int imx_gw5200_stop_port(tids90ub_context_t* ctxt, uint32 port);
static int imx_gw5200_set_port_fsync(tids90ub_context_t* ctxt, uint32 port);
static int imx_gw5200_set_port_mode(tids90ub_context_t* ctxt, uint32 port, tids90ub_sensor_port_mode_t mode);
static int imx_gw5200_apply_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info);
static int imx_gw5200_apply_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure);
static int imx_gw5200_apply_gamma(tids90ub_context_t* ctxt, uint32 port, qcarcam_gamma_config_t* gamma_info);
static int imx_gw5200_apply_hue(tids90ub_context_t* ctxt, uint32 port, float* hue_val);
static int imx_gw5200_apply_saturation(tids90ub_context_t* ctxt, uint32 port, float* saturation_val);
static int imx_gw5200_apply_brightness(tids90ub_context_t* ctxt, uint32 port, float* brightness_val);
static int imx_gw5200_apply_contrast(tids90ub_context_t* ctxt, uint32 port, float* contrast_val);
static int imx_gw5200_set_miror_h(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_h_val);
static int imx_gw5200_set_mirror_v(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_v_val);
static int imx_gw5200_set_vendor_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_vendor_param_t* vendor_param);
static int imx_gw5200_get_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info);
static int imx_gw5200_get_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure);
static int imx_gw5200_get_gamma(tids90ub_context_t* ctxt, uint32 port, qcarcam_gamma_config_t* gamma_info);
static int imx_gw5200_get_hue(tids90ub_context_t* ctxt, uint32 port, float* hue_val);
static int imx_gw5200_get_saturation(tids90ub_context_t* ctxt, uint32 port, float* saturation_val);
static int imx_gw5200_get_brightness(tids90ub_context_t* ctxt, uint32 port, float* brightness_val);
static int imx_gw5200_get_contrast(tids90ub_context_t* ctxt, uint32 port, float* contrast_val);
static int imx_gw5200_get_miror_h(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_h_val);
static int imx_gw5200_get_mirror_v(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_v_val);
static int imx_gw5200_get_vendor_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_vendor_param_t* vendor_param);

static int imx_gw5200_get_ext_isp_property(tids90ub_context_t* ctxt, uint32 port, vendor_ext_property_t* prop);
static int imx_gw5200_set_ext_isp_property(tids90ub_context_t* ctxt, uint32 port, vendor_ext_property_t* prop);

static int imx_gw5200_set_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param);
static int imx_gw5200_get_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param);

#if ENABLE_BULK_WRITEREAD_TESTING
static void imx_gw5200_test_bulk_write_then_read(tids90ub_context_t* ctxt, uint32 port);
#endif

static tids90ub_sensor_t imx_gw5200_info = {
    .detect = imx_gw5200_detect,
    .get_port_cfg = imx_gw5200_get_port_cfg,

    .init_port = imx_gw5200_init_port,
    .start_port = imx_gw5200_start_port,
    .stop_port = imx_gw5200_stop_port,
    .set_port_fsync = imx_gw5200_set_port_fsync,
    .set_port_mode = imx_gw5200_set_port_mode,
    .apply_exposure = imx_gw5200_apply_exposure,
    .apply_hdr_exposure = imx_gw5200_apply_hdr_exposure,
    .set_param = imx_gw5200_set_param,
    .get_param = imx_gw5200_get_param
};


static struct camera_i2c_reg_setting imx_gw5200_reg_setting =
{
    .reg_array = NULL,
    .size = 0,
    .addr_type = CAMERA_I2C_BYTE_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
};

tids90ub_sensor_t* imx_gw5200_get_sensor_info(void)
{
    return &imx_gw5200_info;
}

static int imx_gw5200_detect(tids90ub_context_t* ctxt, uint32 port)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};

    /*read chip id*/
    imx_gw5200_reg_setting.reg_array = read_reg;
    imx_gw5200_reg_setting.size = STD_ARRAY_SIZE(read_reg);

    imx_gw5200_reg_setting.reg_array[0].delay = 0;
    imx_gw5200_reg_setting.reg_array[0].reg_data = 0;
    imx_gw5200_reg_setting.reg_array[0].reg_addr = 0;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &imx_gw5200_reg_setting))) {
        SERR("IMX SER (0x%x) unable to read ID", pSensor->serializer_alias);
        rc = 0;
        //@TODO return -1;
    }

#if 0
    imx_gw5200_reg_setting.reg_array[0].reg_addr = SLAVE_IDENT_PID_REG;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &imx_gw5200_reg_setting))) {
        SERR("IMX424 (0x%x) unable to read ID", pSensor->sensor_alias);
        return -1;
    }

    if (SLAVE_IDENT_PID_ID != imx_gw5200_reg_setting.reg_array[0].reg_data)
    {
        SERR("IMX424(0x%x) read back incorrect ID 0x%x. Expecting 0x%x.",
                pSensor->sensor_alias,
                imx_gw5200_reg_setting.reg_array[0].reg_data,
                SLAVE_IDENT_PID_ID);
        return -1;
    }
#endif
    SWARN("Detected IMX Ser 0x%x", read_reg[0].reg_data);

    pSensor->state = TIDS90UB_SENSOR_STATE_DETECTED;

    return rc;
}

static int imx_gw5200_get_port_cfg(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* p_cfg)
{
    (void)ctxt;

    switch (p_cfg->sensor_id)
    {
    case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI953:
        if (ctxt->config.sensors[port].sensor_mode < IMX424_GW5200_TI953_MODE_MAX)
        {
            p_cfg->sources[0] = imx424_gw5200_ti953_output_mode[ctxt->config.sensors[port].sensor_mode];
        }
        break;
    case TIDS90UB_SENSOR_ID_IMX490_GW5200_TI953:
        p_cfg->sources[0] = imx490_gw5200_ti953_output_mode;
        break;
    case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI951:
        if (ctxt->config.sensors[port].sensor_mode < IMX424_GW5200_TI951_MODE_MAX)
        {
            p_cfg->sources[0] = imx424_gw5200_ti951_output_mode[ctxt->config.sensors[port].sensor_mode];
        }
        break;
    case TIDS90UB_SENSOR_ID_IMX424_GW5200_TI971:
        if (ctxt->config.sensors[port].sensor_mode < IMX424_GW5200_TI971_MODE_MAX)
        {
            p_cfg->sources[0] = imx424_gw5200_ti971_output_mode[ctxt->config.sensors[port].sensor_mode];
        }
        break;
    default:
        p_cfg->sources[0] = imx424_gw5200_ti953_output_mode[IMX424_GW5200_TI953_MODE_DEFAULT];
        break;
    }

    p_cfg->sources[0].channel_info = imx_gw5200_channel_info[port];
    p_cfg->num_sources = 1;
    p_cfg->deser_config = imx_gw5200_tids90ub_init;
    p_cfg->deser_config_size = STD_ARRAY_SIZE(imx_gw5200_tids90ub_init);

    return 0;
}

static int imx_gw5200_init_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_DETECTED == pSensor->state)
    {
#if 0 //@todo
        imx_gw5200_reg_setting.reg_array = imx_gw5200_init;
        imx_gw5200_reg_setting.size = STD_ARRAY_SIZE(imx_gw5200_init);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pSensor->serializer_alias, &imx_gw5200_reg_setting);
        if (rc)
        {
            SERR("Failed to init camera serializer(0x%x)", pSensor->serializer_alias);
            return -1;
        }
#endif
        pSensor->state = TIDS90UB_SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("IMX %d init in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_start_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SHIGH("starting serializer");
#if 0 //@todo
        imx_gw5200_reg_setting.reg_array = imx_gw5200_start;
        imx_gw5200_reg_setting.size = STD_ARRAY_SIZE(imx_gw5200_start);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &imx_gw5200_reg_setting);
        if (rc)
        {
            SERR("serializer 0x%x failed to start", pSensor->serializer_alias);
        }
#endif
    }
    else
    {
        SERR("IMX %d start in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_stop_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_STREAMING == pSensor->state)
    {
#if 0 //@todo
        imx_gw5200_reg_setting.reg_array = imx_gw5200_stop;
        imx_gw5200_reg_setting.size = STD_ARRAY_SIZE(imx_gw5200_stop);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &imx_gw5200_reg_setting)))
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }
#endif
    }
    else
    {
        SERR("IMX %d stop in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_set_port_fsync(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_DETECTED == pSensor->state)
    {
        imx_gw5200_reg_setting.reg_array = imx_gw5200_set_fsync;
        imx_gw5200_reg_setting.size = STD_ARRAY_SIZE(imx_gw5200_set_fsync);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &imx_gw5200_reg_setting);
        if (rc)
        {
            SERR("Failed to set fsync for serializer 0x%x", pSensor->serializer_alias);
        }
    }
    else
    {
        SERR("IMX %d set_port_fsync in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_set_port_mode(tids90ub_context_t* ctxt, uint32 port, tids90ub_sensor_port_mode_t mode)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    switch (mode)
    {
        case IMX_GW5200_MODE_SYNC:
            if (pSensor->port_cfg.sensor_id == TIDS90UB_SENSOR_ID_IMX424_GW5200_TI951 ||
                pSensor->port_cfg.sensor_id == TIDS90UB_SENSOR_ID_IMX424_GW5200_TI971)
            {
                /* Serializer mode change is needed for use with TI9702 & TI960 */
                imx_gw5200_reg_setting.reg_array = imx_gw5200_mode_sel;
                imx_gw5200_reg_setting.size = STD_ARRAY_SIZE(imx_gw5200_mode_sel);
                rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    SENSOR_DEFAULT_ADDR,
                    &imx_gw5200_reg_setting);
                if (rc)
                {
                    SERR("Failed to set synchronous mode for serializer 0x%x", SENSOR_DEFAULT_ADDR);
                }
            }
            break;

        case IMX_GW5200_MODE_BACKW_COMPAT:
            if (pSensor->port_cfg.sensor_id == TIDS90UB_SENSOR_ID_IMX424_GW5200_TI971)
            {
                /* Backwards compatibility mode is needed for use with TI9702 & TI960 */
                imx_gw5200_reg_setting.reg_array = imx_gw5200_backw_comp_sel;
                imx_gw5200_reg_setting.size = STD_ARRAY_SIZE(imx_gw5200_backw_comp_sel);
                rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    SENSOR_DEFAULT_ADDR,
                    &imx_gw5200_reg_setting);
                if (rc)
                {
                    SERR("Failed to set backwards comp mode for serializer 0x%x", SENSOR_DEFAULT_ADDR);
                }
            }
            break;

        default:
            break;
    }

    return rc;
}

static int imx_gw5200_apply_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != exposure_info)
    {
        pSensor->sensor_params.exposure_info = *exposure_info;
        SHIGH("imx_gw5200_apply_exposure: Setting EXPOSURE for port %u , exposure mode = %u ", port, exposure_info->exposure_mode_type);
    }
    else
    {
        SERR("imx_gw5200_apply_exposure: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_apply_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != hdr_exposure)
    {
        pSensor->sensor_params.hdr_exposure_info = *hdr_exposure;
        SHIGH("imx_gw5200_apply_hdr_exposure: Setting HDR EXPOSURE for port %u , exposure mode = %u ", port, hdr_exposure->exposure_mode_type);
    }
    else
    {
        SERR("imx_gw5200_apply_hdr_exposure: param ptr is NULL!");
        rc = -1;
    }
    
    return rc;
}

static int imx_gw5200_apply_gamma(tids90ub_context_t* ctxt, uint32 port, qcarcam_gamma_config_t* gamma_info)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != gamma_info)
    {
        if (QCARCAM_GAMMA_EXPONENT == gamma_info->config_type)
        {
            pSensor->sensor_params.gamma_info.gamma.f_value
                = gamma_info->gamma.f_value;
        }
        else if (QCARCAM_GAMMA_KNEEPOINTS == gamma_info->config_type)
        {
            unsigned int len = gamma_info->gamma.table.length;
            unsigned int* pTable = gamma_info->gamma.table.p_value;
            if (!len || !pTable || len > QCARCAM_MAX_GAMMA_TABLE)
            {
                SERR("imx_gw5200_apply_gamma: Invalid gamma table size (%d) or ptr(%p)", len, pTable);
                rc = -1;
            }
            else
            {
                memcpy(pSensor->sensor_params.gamma_info.gamma.table.p_value, pTable, len * sizeof(*pTable));
                pSensor->sensor_params.gamma_info.gamma.table.length = len;
            }
        }
        else
        {
            SERR("imx_gw5200_apply_gamma: Unsupported gamma config type (%d)", gamma_info->config_type);
            rc = CAMERA_EBADPARM;
        }

        if (CAMERA_SUCCESS == rc)
        {
            pSensor->sensor_params.gamma_info.config_type =
                gamma_info->config_type;

            SHIGH("imx_gw5200_apply_gamma: Setting GAMMA for port %u, gamma config type = %u", port, gamma_info->config_type);
        }
    }
    else
    {
        SERR("imx_gw5200_apply_gamma: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_apply_hue(tids90ub_context_t* ctxt, uint32 port, float* hue_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != hue_val)
    {
        pSensor->sensor_params.hue_value = *hue_val;
        SHIGH("imx_gw5200_apply_hue: Setting HUE for port %u - hue value = %f", port, *hue_val);
    }
    else
    {
        SERR("imx_gw5200_apply_hue: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_apply_saturation(tids90ub_context_t* ctxt, uint32 port, float* saturation_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != saturation_val)
    {
        pSensor->sensor_params.saturation_value = *saturation_val;
        SHIGH("imx_gw5200_apply_saturation: Setting SATURATION for port %u - value = %f", port, *saturation_val);
    }
    else
    {
        SERR("imx_gw5200_apply_saturation: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_apply_brightness(tids90ub_context_t* ctxt, uint32 port, float* brightness_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != brightness_val)
    {
        pSensor->sensor_params.brightness_value = *brightness_val;
        SHIGH("imx_gw5200_apply_brightness: Setting BRIGHTNESS for port %u - value = %f", port, *brightness_val);
    }
    else
    {
        SERR("imx_gw5200_apply_brightness: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_apply_contrast(tids90ub_context_t* ctxt, uint32 port, float* contrast_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != contrast_val)
    {
        pSensor->sensor_params.contrast_value = *contrast_val;
        SHIGH("imx_gw5200_apply_contrast: Setting CONTRAST for port %u - value = %f", port, *contrast_val);
    }
    else
    {
        SERR("imx_gw5200_apply_contrast: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_set_miror_h(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_h_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != mirror_h_val)
    {
        pSensor->sensor_params.horizontal_mirroring = *mirror_h_val;
        SHIGH("imx_gw5200_set_miror_h: Setting HORIZONTAL MIRRORING for port %u - value = %u", port, *mirror_h_val);
    }
    else
    {
        SERR("imx_gw5200_set_miror_h: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_set_mirror_v(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_v_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != mirror_v_val)
    {
        pSensor->sensor_params.vertical_mirroring = *mirror_v_val;
        SHIGH("imx_gw5200_set_mirror_v: Setting VERTICAL MIRRORING for port %u - value = %u", port, *mirror_v_val);
    }
    else
    {
        SERR("imx_gw5200_set_mirror_v: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != exposure_info)
    {
        *exposure_info = pSensor->sensor_params.exposure_info;
        SHIGH("imx_gw5200_get_exposure: Getting EXPOSURE for port %u , exposure mode = %u ", port, exposure_info->exposure_mode_type);
    }
    else
    {
        SERR("imx_gw5200_get_exposure: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != hdr_exposure)
    {
        *hdr_exposure = pSensor->sensor_params.hdr_exposure_info;
        SHIGH("imx_gw5200_get_hdr_exposure: Getting HDR EXPOSURE for port %u , exposure mode = %u ", port, hdr_exposure->exposure_mode_type);
    }
    else
    {
        SERR("imx_gw5200_get_hdr_exposure: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_gamma(tids90ub_context_t* ctxt, uint32 port, qcarcam_gamma_config_t* gamma_info)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != gamma_info)
    {
        /* Get this parameter from the local context */
        if (QCARCAM_GAMMA_EXPONENT == pSensor->sensor_params.gamma_info.config_type &&
            QCARCAM_GAMMA_EXPONENT == gamma_info->config_type)
        {
            gamma_info->gamma.f_value = pSensor->sensor_params.gamma_info.gamma.f_value;
            SHIGH("imx_gw5200_get_gamma: Getting GAMMA config for port %u (QCARCAM_GAMMA_EXPONENT) - value = %f", port, gamma_info->gamma.f_value);
        }
        else if (QCARCAM_GAMMA_KNEEPOINTS == pSensor->sensor_params.gamma_info.config_type &&
                 QCARCAM_GAMMA_KNEEPOINTS == gamma_info->config_type)
        {
            if (gamma_info->gamma.table.length !=
                pSensor->sensor_params.gamma_info.gamma.table.length)
            {
                SERR("imx_gw5200_get_gamma: Gamma table length does not match");
                rc = CAMERA_EBADPARM;
            }
            else if (NULL == gamma_info->gamma.table.p_value)
            {
                SERR("imx_gw5200_get_gamma: Invalid gamma table ptr");
                rc = CAMERA_EBADPARM;
            }
            else
            {
                SHIGH("imx_gw5200_get_gamma: Copying gamma table for port %u - lenght = %d", port, gamma_info->gamma.table.length);
                memcpy(gamma_info->gamma.table.p_value,
                       pSensor->sensor_params.gamma_info.gamma.table.p_value,
                       pSensor->sensor_params.gamma_info.gamma.table.length*sizeof(unsigned int));
            }
        }
        else
        {
            SERR("imx_gw5200_get_gamma: Unsupported Gamma config type (%d)", gamma_info->config_type);
            rc = CAMERA_EBADPARM;
        }
    }

    return rc;
}

static int imx_gw5200_get_hue(tids90ub_context_t* ctxt, uint32 port, float* hue_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != hue_val)
    {
        *hue_val = pSensor->sensor_params.hue_value;
        SHIGH("imx_gw5200_get_hue: Getting HUE for port %u - value = %f", port, *hue_val);
    }
    else
    {
        SERR("imx_gw5200_get_hue: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_saturation(tids90ub_context_t* ctxt, uint32 port, float* saturation_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != saturation_val)
    {
        *saturation_val = pSensor->sensor_params.saturation_value;
        SHIGH("imx_gw5200_get_saturation: Getting SATURATION for port %u - value = %f", port, *saturation_val);
    }
    else
    {
        SERR("imx_gw5200_get_saturation: param ptr is NULL!");
        rc = -1;
    }


    return rc;
}

static int imx_gw5200_get_brightness(tids90ub_context_t* ctxt, uint32 port, float* brightness_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != brightness_val)
    {
        *brightness_val = pSensor->sensor_params.brightness_value;
        SHIGH("imx_gw5200_get_brightness: Getting BRIGHTNESS for port %u - value = %f", port, *brightness_val);
    }
    else
    {
        SERR("imx_gw5200_get_brightness: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_contrast(tids90ub_context_t* ctxt, uint32 port, float* contrast_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != contrast_val)
    {
        *contrast_val = pSensor->sensor_params.contrast_value;
        SHIGH("imx_gw5200_get_contrast: Getting CONTRAST for port %u - value = %f", port, *contrast_val);
    }
    else
    {
        SERR("imx_gw5200_get_contrast: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_miror_h(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_h_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != mirror_h_val)
    {
        *mirror_h_val = pSensor->sensor_params.horizontal_mirroring;
        SHIGH("imx_gw5200_get_miror_h: Getting HORIZONTAL MIRRORING for port %u value = %u", port, *mirror_h_val);
    }
    else
    {
        SERR("imx_gw5200_get_miror_h: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_mirror_v(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_v_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != mirror_v_val)
    {
        *mirror_v_val = pSensor->sensor_params.vertical_mirroring;
        SHIGH("imx_gw5200_get_mirror_v: Getting HORIZONTAL MIRRORING for port %u value = %u", port, *mirror_v_val);
    }
    else
    {
        SERR("imx_gw5200_get_mirror_v: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

#if ENABLE_BULK_WRITEREAD_TESTING
/* Sample code for I2C transaction protocol of type query-response */ 
static void imx_gw5200_test_bulk_write_then_read(tids90ub_context_t* ctxt,  uint32 port)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    //API and query message values are as provided by customer
    unsigned int API_cmd_msg[] = I2C_BULKMODE_TESTING_ARR_API_CMD;
    unsigned int API_ack_response[6] = {};
    unsigned int query_cmd_msg[] = I2C_BULKMODE_TESTING_ARR_QUERY_CMD;
    unsigned int query_ack_response[20] = {};
    struct camera_i2c_bulk_reg_setting write_reg_setting = {};
    struct camera_i2c_bulk_reg_setting read_reg_setting = {};
    uint32_t checksum = 0, msg_id_received;

    write_reg_setting.addr_type = imx_gw5200_reg_setting.addr_type; 
    write_reg_setting.data_type = imx_gw5200_reg_setting.data_type; 
    read_reg_setting.addr_type = imx_gw5200_reg_setting.addr_type; 
    read_reg_setting.data_type = imx_gw5200_reg_setting.data_type; 

    write_reg_setting.reg_addr = 0x0;
    write_reg_setting.reg_data = API_cmd_msg;
    write_reg_setting.size = STD_ARRAY_SIZE(API_cmd_msg);
    SLOW("Data being send (API cmd) to slave with address 0x%x is below", pSensor->sensor_alias);
    for (int i = 0; i< write_reg_setting.size; i++)
    {
        SLOW("    [%d] : 0x%02x", i, write_reg_setting.reg_data[i]);
    }

    read_reg_setting.reg_data = API_ack_response;
    read_reg_setting.size = STD_ARRAY_SIZE(API_ack_response);
    read_reg_setting.reg_addr = 0x00;
    rc = pCtxt->platform_fcn_tbl.i2c_slave_bulk_write_then_read(pCtxt->ctrl, pSensor->sensor_alias,
            &write_reg_setting, &read_reg_setting, TRUE);
    if (rc) {
         SERR("Bulk write then read usecase to slave 0x%0x failed for part 1", pSensor->sensor_alias);
    }
    else {
        SHIGH("Bulk Write then read to slave id 0x%0x was succesful for part 1: API cmd", pSensor->sensor_alias);
        msg_id_received = read_reg_setting.reg_data[I2C_BULKMODE_TESTING_CMD_MSGID_IDX];
        int read_size = read_reg_setting.size - 1;
        for (int i = 0; i < read_size; i++)
        {
            checksum += read_reg_setting.reg_data[i];
            SHIGH("Response seq[%d] : 0x%02x", i, read_reg_setting.reg_data[i]);
        }
        checksum = checksum & 0xFF;
        SHIGH("Response seq[%d] : 0x%02x", read_reg_setting.size, read_reg_setting.reg_data[read_size]);

        //last byte returned should be checksum of sequence
        if (checksum != read_reg_setting.reg_data[read_size])
        {
            SERR("Last byte received 0x%02x is not equal to checksum 0x%02x",
                    read_reg_setting.reg_data[read_size], checksum);
        }
        else
        {
            //Send part 2: query command
            query_cmd_msg[I2C_BULKMODE_TESTING_CMD_MSGID_IDX] = msg_id_received;
            checksum = 0;
            int send_size = STD_ARRAY_SIZE(query_cmd_msg) - 1;
            for (int i=0; i < send_size; i++)
            {
                checksum += query_cmd_msg[i];
            }
            checksum = checksum & 0xFF;
            query_cmd_msg[send_size] = checksum;

            write_reg_setting.reg_data = query_cmd_msg;
            write_reg_setting.size = STD_ARRAY_SIZE(query_cmd_msg);
            SLOW("Data being send (query cmd) to slave with address 0x%x is below", pSensor->sensor_alias);
            for (int i = 0; i < write_reg_setting.size; i++)
            {
                SLOW("    [%d] : 0x%x", i, write_reg_setting.reg_data[i]);
            }

            read_reg_setting.reg_addr = 0x00;
            read_reg_setting.reg_data = query_ack_response;
            read_reg_setting.size = STD_ARRAY_SIZE(query_ack_response);
            rc = pCtxt->platform_fcn_tbl.i2c_slave_bulk_write_then_read(pCtxt->ctrl, pSensor->sensor_alias, &write_reg_setting, &read_reg_setting, TRUE);
            if (rc) {
                 SERR("Bulk write then read usecase failed for part 2");
            }
            else {
                SHIGH("Bulk Write then read to slave id 0x%0x was succesful for part 2: query cmd", pSensor->sensor_alias);
                checksum = 0;
                read_size = read_reg_setting.size - 1;
                for (int i=0; i < read_size; i++) {
                    checksum += read_reg_setting.reg_data[i];
                    SHIGH("Response seq[%d] : 0x%02x", i, read_reg_setting.reg_data[i]);
                }
                checksum = checksum & 0x00FF;
                SHIGH("Response seq[%d] : 0x%02x", read_size, read_reg_setting.reg_data[read_size]);
                //last byte returned should be checksum of sequence
                if (checksum != read_reg_setting.reg_data[read_size])
                {
                    SERR("Last byte received 0x%02x is not equal to checksum 0x%02x",
                            read_reg_setting.reg_data[read_size], checksum);
                }
            }
        }
    }
}
#endif //ENABLE_BULK_WRITEREAD_TESTING

static int imx_gw5200_set_vendor_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_vendor_param_t* vendor_param)
{
    int rc = 0;
    vendor_ext_property_t *p_prop = NULL;

    if (NULL != vendor_param)
    {
        p_prop = (vendor_ext_property_t*)&(vendor_param->data[0]);

        rc = imx_gw5200_set_ext_isp_property(ctxt, port, p_prop);

        if (0 != rc)
        {
            SERR("imx_gw5200_set_vendor_param: setting ISP param failed!");
            rc = -1;
        }
#if ENABLE_BULK_WRITEREAD_TESTING
        imx_gw5200_test_bulk_write_then_read(ctxt, port);
#endif //ENABLE_BULK_WRITEREAD_TESTING
    }
    else
    {
        SERR("imx_gw5200_set_vendor_param: vendor param is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_vendor_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_vendor_param_t* vendor_param)
{
    int rc = 0;
    vendor_ext_property_t *p_prop = NULL;

    if (NULL != vendor_param)
    {
        p_prop = (vendor_ext_property_t*)&(vendor_param->data[0]);

        rc = imx_gw5200_get_ext_isp_property(ctxt, port, p_prop);

        if (0 != rc)
        {
            SERR("imx_gw5200_get_vendor_param: getting ISP param failed!");
            rc = -1;
        }
    }
    else
    {
        SERR("imx_gw5200_get_vendor_param: vendor param is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_set_ext_isp_property(tids90ub_context_t* ctxt, uint32 port, vendor_ext_property_t* p_prop)
{
    int rc = 0;
    CAM_UNUSED(ctxt);
    CAM_UNUSED(port);

    if (NULL != p_prop)
    {
        switch (p_prop->type)
        {
            case VENDOR_EXT_PROP_TEST:
            {
                SHIGH("imx_gw5200_set_ext_isp_property (VENDOR_EXT_PROP_TEST), val = %u", p_prop->value.uint_val);
                break;
            }
            case VENDOR_EXT_PROP_RESET:
            case VENDOR_EXT_PROP_CLEAR_OP_STATUS:
            case VENDOR_EXT_PROP_PIXEL_FORMAT:
            case VENDOR_EXT_PROP_AUTO_HUE:
            case VENDOR_EXT_PROP_AUTO_WHITE_BALANCE:
            case VENDOR_EXT_PROP_WHITE_BALANCE_TEMP:
            case VENDOR_EXT_PROP_EXPOSURE_COMPENSATION:
            case VENDOR_EXT_PROP_BACKLIGHT_COMPENSATION:
            case VENDOR_EXT_PROP_SHARPNESS:
            case VENDOR_EXT_PROP_HDR_ENABLE:
            case VENDOR_EXT_PROP_NOISE_REDUCTION:
            case VENDOR_EXT_PROP_FRAME_RATE:
            case VENDOR_EXT_PROP_FOCAL_LENGTH_H:
            case VENDOR_EXT_PROP_FOCAL_LENGTH_v:
            case VENDOR_EXT_PROP_NUM_EMBEDDED_LINES_TOP:
            case VENDOR_EXT_PROP_NUM_EMBEDDED_LINES_BOTTOM:
            case VENDOR_EXT_PROP_ROI_STATISTICS:
            case VENDOR_EXT_PROP_METADATA_MODES:
            case VENDOR_EXT_PROP_TEMP_SENSOR_0:
            case VENDOR_EXT_PROP_TEMP_SENSOR_1:
            {
                SHIGH("imx_gw5200_set_ext_isp_property: type = %u", p_prop->type);
                break;
            }
            default:
                SERR("imx_gw5200_set_ext_isp_property: unsupported param type = %u", p_prop->type);
                rc = -1;
                break;
        }
    }
    else
    {
        SERR("imx_gw5200_set_ext_isp_property: p_prop is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_get_ext_isp_property(tids90ub_context_t* ctxt, uint32 port, vendor_ext_property_t* p_prop)
{
    int rc = 0;

    CAM_UNUSED(ctxt);
    CAM_UNUSED(port);

    if (NULL != p_prop)
    {
        switch (p_prop->type)
        {
            case VENDOR_EXT_PROP_TEST:
            {
                SHIGH("imx_gw5200_get_ext_isp_property (VENDOR_EXT_PROP_TEST)");
                p_prop->value.uint_val = 0x12345678;
                break;
            }
            case VENDOR_EXT_PROP_RESET:
            case VENDOR_EXT_PROP_CLEAR_OP_STATUS:
            case VENDOR_EXT_PROP_PIXEL_FORMAT:
            case VENDOR_EXT_PROP_AUTO_HUE:
            case VENDOR_EXT_PROP_AUTO_WHITE_BALANCE:
            case VENDOR_EXT_PROP_WHITE_BALANCE_TEMP:
            case VENDOR_EXT_PROP_EXPOSURE_COMPENSATION:
            case VENDOR_EXT_PROP_BACKLIGHT_COMPENSATION:
            case VENDOR_EXT_PROP_SHARPNESS:
            case VENDOR_EXT_PROP_HDR_ENABLE:
            case VENDOR_EXT_PROP_NOISE_REDUCTION:
            case VENDOR_EXT_PROP_FRAME_RATE:
            case VENDOR_EXT_PROP_FOCAL_LENGTH_H:
            case VENDOR_EXT_PROP_FOCAL_LENGTH_v:
            case VENDOR_EXT_PROP_NUM_EMBEDDED_LINES_TOP:
            case VENDOR_EXT_PROP_NUM_EMBEDDED_LINES_BOTTOM:
            case VENDOR_EXT_PROP_ROI_STATISTICS:
            case VENDOR_EXT_PROP_METADATA_MODES:
            case VENDOR_EXT_PROP_TEMP_SENSOR_0:
            case VENDOR_EXT_PROP_TEMP_SENSOR_1:
            {
                SHIGH("imx_gw5200_get_ext_isp_property: type = %u", p_prop->type);
                break;
            }
            default:
                SERR("imx_gw5200_get_ext_isp_property: unsupported param type = %u", p_prop->type);
                rc = -1;
                break;
        }
    }
    else
    {
        SERR("imx_gw5200_get_ext_isp_property: p_prop is NULL!");
        rc = -1;
    }

    return rc;
}

static int imx_gw5200_set_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param)
{
    int rc = 0;

    if (NULL == ctxt)
    {
        SERR("Invalid ctxt");
        return -1;
    }

    if (NULL == p_param)
    {
        SERR("Invalid params");
        return -1;
    }

    switch(param_id)
    {
        case QCARCAM_PARAM_HUE:
            rc = imx_gw5200_apply_hue(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_SATURATION:
            rc = imx_gw5200_apply_saturation(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_GAMMA:
            rc = imx_gw5200_apply_gamma(ctxt, port, (qcarcam_gamma_config_t*)p_param);
            break;
        case QCARCAM_PARAM_BRIGHTNESS:
            rc = imx_gw5200_apply_brightness(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_CONTRAST:
            rc = imx_gw5200_apply_contrast(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_MIRROR_H:
            rc = imx_gw5200_set_miror_h(ctxt, port, (uint32_t*)p_param);
            break;
        case QCARCAM_PARAM_MIRROR_V:
            rc = imx_gw5200_set_mirror_v(ctxt, port, (uint32_t*)p_param);
            break;
        case QCARCAM_PARAM_VENDOR:
            rc = imx_gw5200_set_vendor_param(ctxt, port, (qcarcam_vendor_param_t*)p_param);
            break;
        default:
            SERR("imx_gw5200_set_param. Param %d not supported", param_id);
            rc = -1;
            break;
    }
    return rc;
}

static int imx_gw5200_get_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param)
{
    int rc = 0;

    if (NULL == ctxt)
    {
        SERR("Invalid ctxt");
        return -1;
    }

    if (NULL == p_param)
    {
        SERR("Invalid params");
        return -1;
    }

    switch(param_id)
    {
        case QCARCAM_PARAM_EXPOSURE:
            rc = imx_gw5200_get_exposure(ctxt, port, (sensor_exposure_info_t*)p_param);
            break;
        case QCARCAM_PARAM_HDR_EXPOSURE:
            rc = imx_gw5200_get_hdr_exposure(ctxt, port, (qcarcam_hdr_exposure_config_t*)p_param);
            break;
        case QCARCAM_PARAM_HUE:
            rc = imx_gw5200_get_hue(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_SATURATION:
            rc = imx_gw5200_get_saturation(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_GAMMA:
            rc = imx_gw5200_get_gamma(ctxt, port, (qcarcam_gamma_config_t*)p_param);
            break;
        case QCARCAM_PARAM_BRIGHTNESS:
            rc = imx_gw5200_get_brightness(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_CONTRAST:
            rc = imx_gw5200_get_contrast(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_MIRROR_H:
            rc = imx_gw5200_get_miror_h(ctxt, port, (uint32_t*)p_param);
            break;
        case QCARCAM_PARAM_MIRROR_V:
            rc = imx_gw5200_get_mirror_v(ctxt, port, (uint32_t*)p_param);
            break;
        case QCARCAM_PARAM_VENDOR:
            rc = imx_gw5200_get_vendor_param(ctxt, port, (qcarcam_vendor_param_t*)p_param);
            break;
        default:
            SERR("imx_gw5200_get_param. Param %d not supported", param_id);
            rc = -1;
            break;
    }
    return rc;
}
