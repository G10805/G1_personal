/**
 * @file max9295_loopback_sensor.c
 *
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "max9295_loopback_sensor.h"
#include <string.h>

#define SENSOR_WIDTH    1920
#define SENSOR_HEIGHT   1200

#define CAM_SENSOR_DEFAULT_ADDR         0xE0
#define CAM_EXT_ISP_DEFAULT_ADDR        0xBA

/*CONFIGURATION OPTIONS*/

#define _max9295_loopback_delay_ 0
#define MAX9295_LINK_RESET_DELAY 100000
#define MAX9295_START_DELAY 20000
#define MAX9295_STOP_DELAY 20000

#define CID_VC0        0
#define CID_VC1        4


/*
Reg 1B0 - 1B9
Use ‘crossbar’ in the serializer move bits from 10 to 8, moving last two LSB to MSB

9 .... 0 -> 1098765432 in serialzer (cross bar)
10 | 98765432 (8 bits) in deser (truncate)

Deseralizer is programmed to truncate from 10 to 8 (in MSB). Effective, you get 9 - 2, truncating just the 2 LSB bits
*/

#define CAM_SER_START \
{ \
    { 0x0007, 0xf7, _max9295_loopback_delay_ }, \
    { 0x03f1, 0x05, _max9295_loopback_delay_ }, \
    { 0x03f0, 0x71, _max9295_loopback_delay_ }, \
    { 0x0313, 0x08, _max9295_loopback_delay_ }, \
    { 0x01E5, 0x01, _max9295_loopback_delay_ }, \
    { 0x01C8, 0xE3, _max9295_loopback_delay_ }, \
}

#define CAM_SER_STOP \
{ \
    { 0x01E5, 0x00, _max9295_loopback_delay_ }, \
    { 0x01C8, 0x03, _max9295_loopback_delay_ }, \
}

#define CAM_SER_ADDR_INIT_A \
{ \
    { 0x01C8, 0x03, _max9295_loopback_delay_ }, \
    { 0x01C9, 0x19, _max9295_loopback_delay_ }, \
    { 0x01CA, 0x00, _max9295_loopback_delay_ }, \
    { 0x01CB, 0x00, _max9295_loopback_delay_ }, \
    { 0x01CC, 0x00, _max9295_loopback_delay_ }, \
    { 0x01CD, 0x00, _max9295_loopback_delay_ }, \
    { 0x01CE, 0x1F, _max9295_loopback_delay_ }, \
    { 0x01CF, 0x94, _max9295_loopback_delay_ }, \
    { 0x01D0, 0x26, _max9295_loopback_delay_ }, \
    { 0x01D1, 0x05, _max9295_loopback_delay_ }, \
    { 0x01D2, 0xF5, _max9295_loopback_delay_ }, \
    { 0x01D3, 0x00, _max9295_loopback_delay_ }, \
    { 0x01D4, 0xEC, _max9295_loopback_delay_ }, \
    { 0x01D5, 0x94, _max9295_loopback_delay_ }, \
    { 0x01D6, 0x07, _max9295_loopback_delay_ }, \
    { 0x01D7, 0xDB, _max9295_loopback_delay_ }, \
    { 0x01D8, 0x00, _max9295_loopback_delay_ }, \
    { 0x01D9, 0x0A, _max9295_loopback_delay_ }, \
    { 0x01DA, 0x04, _max9295_loopback_delay_ }, \
    { 0x01DB, 0xD5, _max9295_loopback_delay_ }, \
    { 0x01DC, 0x00, _max9295_loopback_delay_ }, \
    { 0x01DD, 0xEC, _max9295_loopback_delay_ }, \
    { 0x01DE, 0xD6, _max9295_loopback_delay_ }, \
    { 0x01DF, 0x07, _max9295_loopback_delay_ }, \
    { 0x01E0, 0x80, _max9295_loopback_delay_ }, \
    { 0x01E1, 0x00, _max9295_loopback_delay_ }, \
    { 0x01E2, 0x65, _max9295_loopback_delay_ }, \
    { 0x01E3, 0x04, _max9295_loopback_delay_ }, \
    { 0x01E4, 0xB0, _max9295_loopback_delay_ }, \
    { 0x01E5, 0x01, _max9295_loopback_delay_ }, \
    { 0x01E6, 0x06, _max9295_loopback_delay_ }, \
    { 0x01E7, 0x80, _max9295_loopback_delay_ }, \
    { 0x01E8, 0x00, _max9295_loopback_delay_ }, \
    { 0x01E9, 0x04, _max9295_loopback_delay_ }, \
    { 0x01EA, 0x00, _max9295_loopback_delay_ }, \
    { 0x01EB, 0x08, _max9295_loopback_delay_ }, \
    { 0x01EC, 0x80, _max9295_loopback_delay_ }, \
    { 0x01ED, 0x50, _max9295_loopback_delay_ }, \
    { 0x01EE, 0xA0, _max9295_loopback_delay_ }, \
    { 0x01EF, 0x50, _max9295_loopback_delay_ }, \
    { 0x01C8, 0x03, _max9295_loopback_delay_ }, \
    { 0x0007, 0xF7, _max9295_loopback_delay_ }, \
    { 0x0006, 0xBF, _max9295_loopback_delay_ }, \
    { 0x0003, 0x07, _max9295_loopback_delay_ }, \
    { 0x00F0, 0x71, _max9295_loopback_delay_ }, \
}

static int max9295_loopback_detect(max9296_context_t* ctxt, uint32 link);
static int max9295_loopback_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg);
static int max9295_loopback_init_link(max9296_context_t* ctxt, uint32 link);
static int max9295_loopback_start_link(max9296_context_t* ctxt, uint32 link);
static int max9295_loopback_stop_link(max9296_context_t* ctxt, uint32 link);


static max9296_sensor_t max9295_loopback_info = {
    .id = MAXIM_SENSOR_ID_MAX9295_LOOPBACK,
    .detect = max9295_loopback_detect,
    .get_link_cfg = max9295_loopback_get_link_cfg,

    .init_link = max9295_loopback_init_link,
    .start_link = max9295_loopback_start_link,
    .stop_link = max9295_loopback_stop_link,
};

typedef struct
{
    struct camera_i2c_reg_setting cam_ser_reg_setting;
}max9295_loopback_contxt_t;


static struct camera_i2c_reg_array max9295_loopback_gmsl_0[] = CAM_SER_ADDR_INIT_A;
static struct camera_i2c_reg_array max9295_loopback_start_reg[] = CAM_SER_START;
static struct camera_i2c_reg_array max9295_loopback_stop_reg[] = CAM_SER_STOP;


// List of serializer addresses we support
static uint16 supported_ser_addr[] = {0xC4, 0x88, 0x80, 0};

static maxim_pipeline_t max9295_loopback_pipelines[MAX9295_LOOPBACK_MODE_MAX][MAXIM_LINK_MAX] =
{
    [MAX9295_LOOPBACK_FULL_RES] = {
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = QCARCAM_FMT_RGB_888,
                .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                .channel_info = {.vc = 0, .dt = CSI_DT_RGB888, .cid = CID_VC0},
            }
        }
    },
};

max9296_sensor_t* max9295_loopback_get_info(void)
{
    return &max9295_loopback_info;
}

static int max9295_loopback_create_ctxt(max9296_sensor_info_t* pSensor)
{
    int rc = 0;

    if (pSensor->pPrivCtxt)
    {
        SLOW("Ctxt already created");
        return 0;
    }

    if(pSensor->mode >= MAX9295_LOOPBACK_MODE_MAX)
    {
        SERR("Unsupported sensor mode %d", pSensor->mode);
        return CAMERA_EUNSUPPORTED;
    }

    max9295_loopback_contxt_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(max9295_loopback_contxt_t));
    if (pCtxt)
    {
        memset(pCtxt, 0x0, sizeof(*pCtxt));

        pCtxt->cam_ser_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->cam_ser_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        pSensor->pPrivCtxt = pCtxt;
    }

    return rc;
}

static int max9295_loopback_detect(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = (max9296_context_t*)ctxt;
    int rc = 0;
    int i = 0;
    int num_addr = STD_ARRAY_SIZE(supported_ser_addr);
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    sensor_platform_func_table_t* sensor_fcn_tbl = &pCtxt->platform_fcn_tbl;
    max9295_loopback_contxt_t* max9295_loopback_ctxt;

    rc = max9295_loopback_create_ctxt(pSensor);
    if (rc)
    {
        SERR("Failed to create ctxt for link %d", link);
        return rc;
    }

    max9295_loopback_ctxt = pSensor->pPrivCtxt;

    max9295_loopback_ctxt->cam_ser_reg_setting.reg_array = read_reg;
    max9295_loopback_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
    supported_ser_addr[num_addr-1] = pSensor->serializer_alias;

    /* Detect far end serializer */
    for (i = 0; i < num_addr; i++)
    {
        max9295_loopback_ctxt->cam_ser_reg_setting.reg_array[0].reg_addr = MSM_SER_CHIP_ID_REG_ADDR;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, supported_ser_addr[i], &max9295_loopback_ctxt->cam_ser_reg_setting);
        if (!rc)
        {
            pSensor->serializer_addr = supported_ser_addr[i];
	    SENSOR_WARN("** Link %d of detected serializer addr 0x%x", link, pSensor->serializer_addr);
            break;
        }
    }

    if (i == num_addr)
    {
        SENSOR_WARN("No Camera connected to Link %d of MAX9296 0x%x", link, pCtxt->slave_addr);
    }
    else
    {
        struct camera_i2c_reg_array remap_ser[] = {
            {0x0, pSensor->serializer_alias, _max9295_loopback_delay_}
        };

        SENSOR_WARN("Detected Camera connected to Link %d, Ser ID[0x%x]: addr 0x%x",
            link, MSM_SER_CHIP_ID_REG_ADDR, max9295_loopback_ctxt->cam_ser_reg_setting.reg_array[0].reg_data);

        max9295_loopback_ctxt->cam_ser_reg_setting.reg_array = remap_ser;
        max9295_loopback_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl,pSensor->serializer_addr, &max9295_loopback_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to change serializer address (0x%x) of MAX9296 0x%x Link %d",
                pSensor->serializer_addr, pCtxt->slave_addr, link);
            return rc;
        }
        max9295_loopback_ctxt->cam_ser_reg_setting.reg_array = max9295_loopback_gmsl_0;
        max9295_loopback_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_loopback_gmsl_0);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &max9295_loopback_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            return rc;
        }
    }

    return rc;
}

static int max9295_loopback_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg)
{
    int rc = 0;
    unsigned int mode;

    mode = ctxt->max9296_sensors[link].mode;

    if(mode < MAX9295_LOOPBACK_MODE_MAX)
    {
        p_cfg->num_pipelines = 1;
        p_cfg->pipelines[0] = max9295_loopback_pipelines[mode][link];
    }
    else
    {
        p_cfg->num_pipelines = 0;
        SERR("Invalid sensor mode %d", mode);
        rc = CAMERA_EBADPARM;
    }
    return rc;
}

static int max9295_loopback_init_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    int rc = 0;

    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("max9296 loopback %d init in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}


static int max9295_loopback_start_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    max9295_loopback_contxt_t* max9295_loopback_ctxt = pSensor->pPrivCtxt;
    int rc;

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SHIGH("starting max9295 loopback serializer");
        max9295_loopback_ctxt->cam_ser_reg_setting.reg_array = max9295_loopback_start_reg;
        max9295_loopback_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_loopback_start_reg);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &max9295_loopback_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("max9295 loopback serializer 0x%x failed to start", pSensor->serializer_alias);
        }
        else
        {
            pSensor->state = SENSOR_STATE_STREAMING;
        }
    }
    else
    {
        SERR("max9295 loopback %d start in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}

static int max9295_loopback_stop_link(max9296_context_t* ctxt, uint32 link)
{
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    max9295_loopback_contxt_t* max9295_loopback_ctxt = pSensor->pPrivCtxt;
    int rc;

    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        max9295_loopback_ctxt->cam_ser_reg_setting.reg_array = max9295_loopback_stop_reg;
        max9295_loopback_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_loopback_stop_reg);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &max9295_loopback_ctxt->cam_ser_reg_setting)))
        {
            SERR("Failed to stop max9295 loopback serializer(0x%x)", pSensor->serializer_alias);
        }

        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("max9295 loopback %d stop in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }

    return rc;
}
