/**
 * @file x3a_ov491.c
 *
 * Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>
#include "x3a_ov491.h"
#include "vendor_ext_properties.h"

#define INIT_DESER \
{ \
    { 0x5B, 0x30, _tids90ub_delay_}, \
    { 0x5d, X3A_OV491_SENSOR_DEFAULT_ADDR, _tids90ub_delay_ }, \
}

#define BC_GPIO1_EN \
{ \
    { 0x0E, 0x20, _x3a_ov491_delay_}, \
    { 0x0D, 0x20, _x3a_ov491_delay_}, \
}

#define INIT_x3a_ov491 {}
#define START_x3a_ov491 {}
#define STOP_x3a_ov491 {}

static struct camera_i2c_reg_array x3a_ov491_tids90ub_init[] = INIT_DESER;

static struct camera_i2c_reg_array x3a_ov491_set_fsync[] = BC_GPIO1_EN;

#if 0 //@todo
static struct camera_i2c_reg_array x3a_ov491_init[] = INIT_x3a_ov491;
static struct camera_i2c_reg_array x3a_ov491_start[] = START_x3a_ov491;
static struct camera_i2c_reg_array x3a_ov491_stop[] = STOP_x3a_ov491;
#endif

static img_src_channel_info_t x3a_ov491_channel_info[TIDS90UB_PORT_MAX] =
{
    {.vc = X3A_OV491_VC0, .dt = X3A_OV491_SENSOR_DT, .cid = X3A_OV491_CID_VC0},
    {.vc = X3A_OV491_VC1, .dt = X3A_OV491_SENSOR_DT, .cid = X3A_OV491_CID_VC1},
    {.vc = X3A_OV491_VC2, .dt = X3A_OV491_SENSOR_DT, .cid = X3A_OV491_CID_VC2},
    {.vc = X3A_OV491_VC3, .dt = X3A_OV491_SENSOR_DT, .cid = X3A_OV491_CID_VC3}
};

static img_src_mode_t x3a_ov491_ti935_output_mode[X3A_OV491_TI935_MODE_MAX] =
{
  [X3A_OV491_TI935_MODE_2MP] =
  {
      .fmt = X3A_OV491_SENSOR_FORMAT,
      .res = {.width = X3A_OV491_SENSOR_WIDTH, .height = X3A_OV491_SENSOR_HEIGHT, .fps = 30.0f},
  },
};

static int x3a_ov491_detect(tids90ub_context_t* ctxt, uint32 port);
static int x3a_ov491_get_port_cfg(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* p_cfg);
static int x3a_ov491_init_port(tids90ub_context_t* ctxt, uint32 port);
static int x3a_ov491_start_port(tids90ub_context_t* ctxt, uint32 port);
static int x3a_ov491_stop_port(tids90ub_context_t* ctxt, uint32 port);
static int x3a_ov491_set_port_fsync(tids90ub_context_t* ctxt, uint32 port);
static int x3a_ov491_apply_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info);
static int x3a_ov491_apply_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure);
static int x3a_ov491_apply_gamma(tids90ub_context_t* ctxt, uint32 port, qcarcam_gamma_config_t* gamma_info);
static int x3a_ov491_apply_hue(tids90ub_context_t* ctxt, uint32 port, float* hue_val);
static int x3a_ov491_apply_saturation(tids90ub_context_t* ctxt, uint32 port, float* saturation_val);
static int x3a_ov491_apply_brightness(tids90ub_context_t* ctxt, uint32 port, float* brightness_val);
static int x3a_ov491_apply_contrast(tids90ub_context_t* ctxt, uint32 port, float* contrast_val);
static int x3a_ov491_set_miror_h(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_h_val);
static int x3a_ov491_set_mirror_v(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_v_val);
static int x3a_ov491_set_vendor_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_vendor_param_t* vendor_param);
static int x3a_ov491_get_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info);
static int x3a_ov491_get_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure);
static int x3a_ov491_get_gamma(tids90ub_context_t* ctxt, uint32 port, qcarcam_gamma_config_t* gamma_info);
static int x3a_ov491_get_hue(tids90ub_context_t* ctxt, uint32 port, float* hue_val);
static int x3a_ov491_get_saturation(tids90ub_context_t* ctxt, uint32 port, float* saturation_val);
static int x3a_ov491_get_brightness(tids90ub_context_t* ctxt, uint32 port, float* brightness_val);
static int x3a_ov491_get_contrast(tids90ub_context_t* ctxt, uint32 port, float* contrast_val);
static int x3a_ov491_get_miror_h(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_h_val);
static int x3a_ov491_get_mirror_v(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_v_val);
static int x3a_ov491_get_vendor_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_vendor_param_t* vendor_param);
static int x3a_ov491_set_ext_isp_property(tids90ub_context_t* ctxt, uint32 port, vendor_ext_property_t* p_prop);
static int x3a_ov491_get_ext_isp_property(tids90ub_context_t* ctxt, uint32 port, vendor_ext_property_t* p_prop);

static int x3a_ov491_set_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param);
static int x3a_ov491_get_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param);

static tids90ub_sensor_t x3a_ov491_info = {
    .detect = x3a_ov491_detect,
    .get_port_cfg = x3a_ov491_get_port_cfg,
    .init_port = x3a_ov491_init_port,
    .start_port = x3a_ov491_start_port,
    .stop_port = x3a_ov491_stop_port,
    .set_port_fsync = x3a_ov491_set_port_fsync,
    .apply_exposure = x3a_ov491_apply_exposure,
    .apply_hdr_exposure = x3a_ov491_apply_hdr_exposure,
    .set_param = x3a_ov491_set_param,
    .get_param = x3a_ov491_get_param
};

static struct camera_i2c_reg_setting x3a_ov491_reg_setting =
{
    .reg_array = NULL,
    .size = 0,
    .addr_type = CAMERA_I2C_BYTE_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
};

tids90ub_sensor_t* x3a_ov491_get_sensor_info(void)
{
    return &x3a_ov491_info;
}

static int x3a_ov491_detect(tids90ub_context_t* ctxt, uint32 port)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = (tids90ub_context_t*)ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};

    /*read chip id*/
    x3a_ov491_reg_setting.reg_array = read_reg;
    x3a_ov491_reg_setting.size = STD_ARRAY_SIZE(read_reg);

    x3a_ov491_reg_setting.reg_array[0].delay = 0;
    x3a_ov491_reg_setting.reg_array[0].reg_data = 0;
    x3a_ov491_reg_setting.reg_array[0].reg_addr = 0;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &x3a_ov491_reg_setting))) {
        SERR("X3A SER (0x%x) unable to read ID", pSensor->serializer_alias);
        rc = 0;
        //@TODO return -1;
    }

#if 0
    x3a_ov491_reg_setting.reg_array[0].reg_addr = SLAVE_IDENT_PID_REG;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &x3a_ov491_reg_setting))) {
        SERR("X3A (0x%x) unable to read ID", pSensor->sensor_alias);
        return -1;
    }

    if (SLAVE_IDENT_PID_ID != x3a_ov491_reg_setting.reg_array[0].reg_data)
    {
        SERR("X3A(0x%x) read back incorrect ID 0x%x. Expecting 0x%x.",
                pSensor->sensor_alias,
                x3a_ov491_reg_setting.reg_array[0].reg_data,
                SLAVE_IDENT_PID_ID);
        return -1;
    }
#endif
    SWARN("Detected X3A Ser 0x%x", read_reg[0].reg_data);

    pSensor->state = TIDS90UB_SENSOR_STATE_DETECTED;

    return rc;
}

static int x3a_ov491_get_port_cfg(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* p_cfg)
{
    (void)ctxt;

    switch (p_cfg->sensor_id)
    {
    case TIDS90UB_SENSOR_ID_X3A_OV491_TI935:
        if (ctxt->config.sensors[port].sensor_mode < X3A_OV491_TI935_MODE_MAX)
        {
            p_cfg->sources[0] = x3a_ov491_ti935_output_mode[ctxt->config.sensors[port].sensor_mode];
        }
        break;
    default:
        p_cfg->sources[0] = x3a_ov491_ti935_output_mode[X3A_OV491_TI935_MODE_DEFAULT];
        break;
    }

    p_cfg->sources[0].channel_info = x3a_ov491_channel_info[port];
    p_cfg->num_sources = 1;

    p_cfg->deser_config = x3a_ov491_tids90ub_init;
    p_cfg->deser_config_size = STD_ARRAY_SIZE(x3a_ov491_tids90ub_init);

    return 0;
}

static int x3a_ov491_init_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_DETECTED == pSensor->state)
    {
#if 0 //@todo
        x3a_ov491_reg_setting.reg_array = x3a_ov491_init;
        x3a_ov491_reg_setting.size = STD_ARRAY_SIZE(x3a_ov491_init);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pSensor->serializer_alias, &x3a_ov491_reg_setting);
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
        SERR("X3A %d init in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_start_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SHIGH("starting serializer");
#if 0 //@todo
        x3a_ov491_reg_setting.reg_array = x3a_ov491_start;
        x3a_ov491_reg_setting.size = STD_ARRAY_SIZE(x3a_ov491_start);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &x3a_ov491_reg_setting);
        if (rc)
        {
            SERR("serializer 0x%x failed to start", pSensor->serializer_alias);
        }
#endif
    }
    else
    {
        SERR("X3A %d start in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_stop_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_STREAMING == pSensor->state)
    {
#if 0 //@todo
        x3a_ov491_reg_setting.reg_array = x3a_ov491_stop;
        x3a_ov491_reg_setting.size = STD_ARRAY_SIZE(x3a_ov491_stop);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &x3a_ov491_reg_setting)))
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }
#endif
    }
    else
    {
        SERR("X3A %d stop in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_set_port_fsync(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc = 0;

    if (TIDS90UB_SENSOR_STATE_DETECTED == pSensor->state)
    {
        x3a_ov491_reg_setting.reg_array = x3a_ov491_set_fsync;
        x3a_ov491_reg_setting.size = STD_ARRAY_SIZE(x3a_ov491_set_fsync);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &x3a_ov491_reg_setting);
        if (rc)
        {
            SERR("Failed to set fsync for serializer 0x%x", pSensor->serializer_alias);
        }
    }
    else
    {
        SERR("X3A %d set_port_fsync in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_apply_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != exposure_info)
    {
        pSensor->sensor_params.exposure_info = *exposure_info;
        SHIGH("x3a_ov491_apply_exposure: Setting EXPOSURE for port %u , exposure mode = %u ", port, exposure_info->exposure_mode_type);
    }
    else
    {
        SERR("x3a_ov491_apply_exposure: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_apply_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != hdr_exposure)
    {
        pSensor->sensor_params.hdr_exposure_info = *hdr_exposure;
        SHIGH("x3a_ov491_apply_hdr_exposure: Setting HDR EXPOSURE for port %u , exposure mode = %u ", port, hdr_exposure->exposure_mode_type);
    }
    else
    {
        SERR("x3a_ov491_apply_hdr_exposure: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_apply_gamma(tids90ub_context_t* ctxt, uint32 port, qcarcam_gamma_config_t* gamma_info)
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
                SERR("x3a_ov491_apply_gamma: Invalid gamma table size (%d) or ptr(%p)", len, pTable);
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
            SERR("x3a_ov491_apply_gamma: Unsupported gamma config type (%d)", gamma_info->config_type);
            rc = CAMERA_EBADPARM;
        }

        if (CAMERA_SUCCESS == rc)
        {
            pSensor->sensor_params.gamma_info.config_type =
                gamma_info->config_type;

            SHIGH("x3a_ov491_apply_gamma: Setting GAMMA for port %u, gamma config type = %u", port, gamma_info->config_type);
        }
    }
    else
    {
        SERR("x3a_ov491_apply_gamma: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_apply_hue(tids90ub_context_t* ctxt, uint32 port, float* hue_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != hue_val)
    {
        pSensor->sensor_params.hue_value = *hue_val;
        SHIGH("x3a_ov491_apply_hue: Setting HUE for port %u - hue value = %f", port, *hue_val);
    }
    else
    {
        SERR("x3a_ov491_apply_hue: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_apply_saturation(tids90ub_context_t* ctxt, uint32 port, float* saturation_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != saturation_val)
    {
        pSensor->sensor_params.saturation_value = *saturation_val;
        SHIGH("x3a_ov491_apply_saturation: Setting SATURATION for port %u - value = %f", port, *saturation_val);
    }
    else
    {
        SERR("x3a_ov491_apply_saturation: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_apply_brightness(tids90ub_context_t* ctxt, uint32 port, float* brightness_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != brightness_val)
    {
        pSensor->sensor_params.brightness_value = *brightness_val;
        SHIGH("x3a_ov491_apply_brightness: Setting BRIGHTNESS for port %u - value = %f", port, *brightness_val);
    }
    else
    {
        SERR("x3a_ov491_apply_brightness: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_apply_contrast(tids90ub_context_t* ctxt, uint32 port, float* contrast_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != contrast_val)
    {
        pSensor->sensor_params.contrast_value = *contrast_val;
                SHIGH("x3a_ov491_apply_contrast: Setting CONTRAST for port %u - value = %f", port, *contrast_val);
    }
    else
    {
        SERR("x3a_ov491_apply_contrast: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_set_miror_h(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_h_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != mirror_h_val)
    {
        pSensor->sensor_params.horizontal_mirroring = *mirror_h_val;
        SHIGH("x3a_ov491_set_miror_h: Setting HORIZONTAL MIRRORING for port %u - value = %u", port, *mirror_h_val);
    }
    else
    {
        SERR("x3a_ov491_set_miror_h: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_set_mirror_v(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_v_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    /* Store the value in local context */
    if (NULL != mirror_v_val)
    {
        pSensor->sensor_params.vertical_mirroring = *mirror_v_val;
        SHIGH("x3a_ov491_set_mirror_v: Setting VERTICAL MIRRORING for port %u - value = %u", port, *mirror_v_val);
    }
    else
    {
        SERR("x3a_ov491_set_mirror_v: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != exposure_info)
    {
        *exposure_info = pSensor->sensor_params.exposure_info;
        SHIGH("x3a_ov491_get_exposure: Getting EXPOSURE for port %u , exposure mode = %u ", port, exposure_info->exposure_mode_type);
    }
    else
    {
        SERR("x3a_ov491_get_exposure: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != hdr_exposure)
    {
        *hdr_exposure = pSensor->sensor_params.hdr_exposure_info;
        SHIGH("x3a_ov491_get_hdr_exposure: Getting HDR EXPOSURE for port %u , exposure mode = %u ", port, hdr_exposure->exposure_mode_type);
    }
    else
    {
        SERR("x3a_ov491_get_hdr_exposure: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_gamma(tids90ub_context_t* ctxt, uint32 port, qcarcam_gamma_config_t* gamma_info)
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
            SHIGH("x3a_ov491_get_gamma: Getting GAMMA config for port %u (QCARCAM_GAMMA_EXPONENT) - value = %f", port, gamma_info->gamma.f_value);
        }
        else if (QCARCAM_GAMMA_KNEEPOINTS == pSensor->sensor_params.gamma_info.config_type &&
                 QCARCAM_GAMMA_KNEEPOINTS == gamma_info->config_type)
        {
            if (gamma_info->gamma.table.length !=
                pSensor->sensor_params.gamma_info.gamma.table.length)
            {
                SERR("x3a_ov491_get_gamma: Gamma table length does not match");
                rc = CAMERA_EBADPARM;
            }
            else if (NULL == gamma_info->gamma.table.p_value)
            {
                SERR("x3a_ov491_get_gamma: Invalid gamma table ptr");
                rc = CAMERA_EBADPARM;
            }
            else
            {
                SHIGH("x3a_ov491_get_gamma: Copying gamma table for port %u - lenght = %d", port, gamma_info->gamma.table.length);
                memcpy(gamma_info->gamma.table.p_value,
                       pSensor->sensor_params.gamma_info.gamma.table.p_value,
                       pSensor->sensor_params.gamma_info.gamma.table.length*sizeof(unsigned int));
            }
        }
        else
        {
            SERR("x3a_ov491_get_gamma: Unsupported Gamma config type (%d)", gamma_info->config_type);
            rc = CAMERA_EBADPARM;
        }
    }

    return rc;
}

static int x3a_ov491_get_hue(tids90ub_context_t* ctxt, uint32 port, float* hue_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != hue_val)
    {
        *hue_val = pSensor->sensor_params.hue_value;
                SHIGH("x3a_ov491_get_hue: Getting HUE for port %u - value = %f", port, *hue_val);
    }
    else
    {
        SERR("x3a_ov491_get_hue: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_saturation(tids90ub_context_t* ctxt, uint32 port, float* saturation_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != saturation_val)
    {
        *saturation_val = pSensor->sensor_params.saturation_value;
        SHIGH("x3a_ov491_get_saturation: Getting SATURATION for port %u - value = %f", port, *saturation_val);
    }
    else
    {
        SERR("x3a_ov491_get_saturation: param ptr is NULL!");
        rc = -1;
    }


    return rc;
}

static int x3a_ov491_get_brightness(tids90ub_context_t* ctxt, uint32 port, float* brightness_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != brightness_val)
    {
        *brightness_val = pSensor->sensor_params.brightness_value;
        SHIGH("x3a_ov491_get_brightness: Getting BRIGHTNESS for port %u - value = %f", port, *brightness_val);
    }
    else
    {
        SERR("x3a_ov491_get_brightness: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_contrast(tids90ub_context_t* ctxt, uint32 port, float* contrast_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != contrast_val)
    {
        *contrast_val = pSensor->sensor_params.contrast_value;
        SHIGH("x3a_ov491_get_contrast: Getting CONTRAST for port %u - value = %f", port, *contrast_val);
    }
    else
    {
        SERR("x3a_ov491_get_contrast: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_miror_h(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_h_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != mirror_h_val)
    {
        *mirror_h_val = pSensor->sensor_params.horizontal_mirroring;
        SHIGH("x3a_ov491_get_miror_h: Getting HORIZONTAL MIRRORING for port %u value = %u", port, *mirror_h_val);
    }
    else
    {
        SERR("x3a_ov491_get_miror_h: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_mirror_v(tids90ub_context_t* ctxt, uint32 port, uint32_t* mirror_v_val)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (NULL != mirror_v_val)
    {
        *mirror_v_val = pSensor->sensor_params.vertical_mirroring;
        SHIGH("x3a_ov491_get_mirror_v: Getting HORIZONTAL MIRRORING for port %u value = %u", port, *mirror_v_val);
    }
    else
    {
        SERR("x3a_ov491_get_mirror_v: param ptr is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_set_vendor_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_vendor_param_t* vendor_param)
{
    int rc = 0;
    vendor_ext_property_t *p_prop = NULL;

    if (NULL != vendor_param)
    {
        p_prop = (vendor_ext_property_t*)&(vendor_param->data[0]);

        rc = x3a_ov491_set_ext_isp_property(ctxt, port, p_prop);

        if (0 != rc)
        {
            SERR("x3a_ov491_set_vendor_param: setting ISP param failed!");
            rc = -1;
        }
    }
    else
    {
        SERR("x3a_ov491_set_vendor_param: vendor param is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_vendor_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_vendor_param_t* vendor_param)
{
    int rc = 0;
    vendor_ext_property_t *p_prop = NULL;

    if (NULL != vendor_param)
    {
        p_prop = (vendor_ext_property_t*)&(vendor_param->data[0]);

        rc = x3a_ov491_get_ext_isp_property(ctxt, port, p_prop);

        if (0 != rc)
        {
            SERR("x3a_ov491_get_vendor_param: getting ISP param failed!");
            rc = -1;
        }
    }
    else
    {
        SERR("x3a_ov491_get_vendor_param: vendor param is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_set_ext_isp_property(tids90ub_context_t* ctxt, uint32 port, vendor_ext_property_t* p_prop)
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
                SHIGH("x3a_ov491_set_ext_isp_property (VENDOR_EXT_PROP_TEST), val = %u", p_prop->value.uint_val);
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
                SHIGH("x3a_ov491_set_ext_isp_property: type = %u", p_prop->type);
                break;
            }
            default:
                SERR("x3a_ov491_set_ext_isp_property: unsupported param type = %u", p_prop->type);
                rc = -1;
                break;
        }
    }
    else
    {
        SERR("x3a_ov491_set_ext_isp_property: p_prop is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_get_ext_isp_property(tids90ub_context_t* ctxt, uint32 port, vendor_ext_property_t* p_prop)
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
                SHIGH("x3a_ov491_get_ext_isp_property (VENDOR_EXT_PROP_TEST)");
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
                SHIGH("x3a_ov491_get_ext_isp_property: type = %u", p_prop->type);
                break;
            }
            default:
                SERR("x3a_ov491_get_ext_isp_property: unsupported param type = %u", p_prop->type);
                rc = -1;
                break;
        }
    }
    else
    {
        SERR("x3a_ov491_get_ext_isp_property: p_prop is NULL!");
        rc = -1;
    }

    return rc;
}

static int x3a_ov491_set_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param)
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
            rc = x3a_ov491_apply_hue(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_SATURATION:
            rc = x3a_ov491_apply_saturation(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_GAMMA:
            rc = x3a_ov491_apply_gamma(ctxt, port, (qcarcam_gamma_config_t*)p_param);
            break;
        case QCARCAM_PARAM_BRIGHTNESS:
            rc = x3a_ov491_apply_brightness(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_CONTRAST:
            rc = x3a_ov491_apply_contrast(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_MIRROR_H:
            rc = x3a_ov491_set_miror_h(ctxt, port, (uint32_t*)p_param);
            break;
        case QCARCAM_PARAM_MIRROR_V:
            rc = x3a_ov491_set_mirror_v(ctxt, port, (uint32_t*)p_param);
            break;
        case QCARCAM_PARAM_VENDOR:
            rc = x3a_ov491_set_vendor_param(ctxt, port, (qcarcam_vendor_param_t*)p_param);
            break;
        default:
            SERR("x3a_ov491_set_param. Param (%d) not supported ", param_id);
            rc = -1;
            break;
    }
    return rc;
}

static int x3a_ov491_get_param(tids90ub_context_t* ctxt, uint32 port, qcarcam_param_t param_id, void* p_param)
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
            rc = x3a_ov491_get_exposure(ctxt, port, (sensor_exposure_info_t*)p_param);
            break;
        case QCARCAM_PARAM_HDR_EXPOSURE:
            rc = x3a_ov491_get_hdr_exposure(ctxt, port, (qcarcam_hdr_exposure_config_t*)p_param);
            break;
        case QCARCAM_PARAM_HUE:
            rc = x3a_ov491_get_hue(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_SATURATION:
            rc = x3a_ov491_get_saturation(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_GAMMA:
            rc = x3a_ov491_get_gamma(ctxt, port, (qcarcam_gamma_config_t*)p_param);
            break;
        case QCARCAM_PARAM_BRIGHTNESS:
            rc = x3a_ov491_get_brightness(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_CONTRAST:
            rc = x3a_ov491_get_contrast(ctxt, port, (float*)p_param);
            break;
        case QCARCAM_PARAM_MIRROR_H:
            rc = x3a_ov491_get_miror_h(ctxt, port, (uint32_t*)p_param);
            break;
        case QCARCAM_PARAM_MIRROR_V:
            rc = x3a_ov491_get_mirror_v(ctxt, port, (uint32_t*)p_param);
            break;
        case QCARCAM_PARAM_VENDOR:
            rc = x3a_ov491_get_vendor_param(ctxt, port, (qcarcam_vendor_param_t*)p_param);
            break;
        default:
            SERR("x3a_ov491_get_param. Param (%d) not supported", param_id);
            rc = -1;
            break;
    }
    return rc;
}
