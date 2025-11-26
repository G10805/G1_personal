/**
 * @file ar0820_ext_isp_gw5300.c
 *
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ar0820_ext_isp_gw5300.h"

#define INIT_SER \
{\
    { 0x0002,0x03,  _ar0820_ext_isp_gw5300_delay_}, \
    { 0x0308,0x64,  _ar0820_ext_isp_gw5300_delay_}, \
    { 0x0311,0x40,  _ar0820_ext_isp_gw5300_delay_}, \
    { 0x0318,0x5E,  _ar0820_ext_isp_gw5300_delay_}, \
}

#define START_SER \
{ \
    { 0x0002,0x43,  _ar0820_ext_isp_gw5300_delay_}, \
}

#define STOP_SER \
{\
    { 0x0002,0x03,  _ar0820_ext_isp_gw5300_delay_}, \
}

static struct camera_i2c_reg_array ar0820_ext_isp_gw5300_init_reg[] = INIT_SER;
static struct camera_i2c_reg_array ar0820_ext_isp_gw5300_start_reg[] = START_SER;
static struct camera_i2c_reg_array ar0820_ext_isp_gw5300_stop_reg[] = STOP_SER;

static maxim_pipeline_t ar0820_ext_isp_gw5300_isp_pipelines[AR0820_EXT_ISP_GW5300_MODE_MAX] =
{
    [AR0820_EXT_ISP_GW5300_MODE_8MP] =
    {
        .id = MAXIM_PIPELINE_Z,
        .mode =
        {
            .fmt = SENSOR_FORMAT,
            .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
            .channel_info = {.vc = 0, .dt = SENSOR_DT, .cid = 0,},
        }
    },
};

static int ar0820_ext_isp_gw5300_detect(max96712_context_t* ctxt, uint32 link);
static int ar0820_ext_isp_gw5300_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg);
static int ar0820_ext_isp_gw5300_init_link(max96712_context_t* ctxt, uint32 link);
static int ar0820_ext_isp_gw5300_start_link(max96712_context_t* ctxt, uint32 link);
static int ar0820_ext_isp_gw5300_stop_link(max96712_context_t* ctxt, uint32 link);

static max96712_sensor_t ar0820_ext_isp_gw5300_info = {
    .id = MAXIM_SENSOR_ID_AR0820_EXT_ISP_GW5300,
    .detect = ar0820_ext_isp_gw5300_detect,
    .get_link_cfg = ar0820_ext_isp_gw5300_get_link_cfg,

    .init_link = ar0820_ext_isp_gw5300_init_link,
    .start_link = ar0820_ext_isp_gw5300_start_link,
    .stop_link = ar0820_ext_isp_gw5300_stop_link,
};


max96712_sensor_t* ar0820_ext_isp_gw5300_get_sensor_info(void)
{
    return &ar0820_ext_isp_gw5300_info;
}

typedef struct
{
    struct camera_i2c_reg_setting ar0820_ext_isp_gw5300_reg_setting;
}ar0820_ext_isp_gw5300_contxt_t;

static int ar0820_ext_isp_gw5300_create_ctxt(max96712_sensor_info_t* pSensor)
{
    int rc = 0;

    if (pSensor->pPrivCtxt)
    {
        SLOW("Ctxt already created");
        rc = 0;
    }
    else if (pSensor->mode >= AR0820_EXT_ISP_GW5300_MODE_MAX)
    {
        SERR("Unsupported sensor mode %d", pSensor->mode);
        rc = -1;
    }
    else
    {
        ar0820_ext_isp_gw5300_contxt_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(ar0820_ext_isp_gw5300_contxt_t));
        if (pCtxt)
        {
            memset(pCtxt, 0x0, sizeof(*pCtxt));

            pCtxt->ar0820_ext_isp_gw5300_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
            pCtxt->ar0820_ext_isp_gw5300_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

            pSensor->pPrivCtxt = pCtxt;
        }
        else
        {
            SERR("Failed to allocate sensor context");
            rc = -1;
        }
    }
    return rc;
}

static int ar0820_ext_isp_gw5300_detect(max96712_context_t* ctxt, uint32 link)
{
    SHIGH("ar0820_ext_isp_gw5300_detect() E");
    int rc = 0;
    int i = 0;
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    uint16 supported_ser_addr[] = {0xC4, 0};// List of serializer addresses we support
    int num_addr = STD_ARRAY_SIZE(supported_ser_addr);
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};
    ar0820_ext_isp_gw5300_contxt_t* ar0820_ext_isp_gw5300_ctxt;

    rc = ar0820_ext_isp_gw5300_create_ctxt(pSensor);
    if (rc)
    {
        SERR("Failed to create ctxt for link %d", link);
        rc = -1;
    }
    else 
    { 
        ar0820_ext_isp_gw5300_ctxt = pSensor->pPrivCtxt;

        /*read chip id for serializer*/
        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array = read_reg;
        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.size = STD_ARRAY_SIZE(read_reg);
        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array[0].delay = 0;
        
        supported_ser_addr[num_addr-1] = pSensor->serializer_alias;
        
        /* Detect far end serializer */
        for (i = 0; i < num_addr; i++)
        {
            ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array[0].reg_addr = MSM_SER_CHIP_ID_REG_ADDR;
            rc = pCtxt->platform_fcn_tbl.i2c_slave_read(pCtxt->ctrl, supported_ser_addr[i], &ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting);
            if (!rc)
            {
                pSensor->serializer_addr = supported_ser_addr[i];
                break;
            }
        }
        if (i == num_addr)
        {
            SENSOR_WARN("No Camera connected to Link %d of max96712 0x%x", link, pCtxt->slave_addr);
        }
        else if (pSensor->serializer_alias == pSensor->serializer_addr)
        {
            SENSOR_WARN("LINK %d already re-mapped 0x%x", link, pSensor->serializer_addr);
            rc = 0;
        }
        else
        {
            /*remmap slave address to new alias*/
            struct camera_i2c_reg_array remap_ser[] = {
                {0x0, pSensor->serializer_alias, _ar0820_ext_isp_gw5300_delay_}
            };
            ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array = remap_ser;
            ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.size = STD_ARRAY_SIZE(remap_ser);
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pSensor->serializer_addr,
                    &ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting);
            if (rc)
            {
                SERR("Failed to change serializer address (0x%x) of max96712 0x%x Link %d",
                    pSensor->serializer_addr, pCtxt->slave_addr, link);
                rc = -1;
            }
            if (!rc)
            {
                /*read chip id for serializer*/
                ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array = read_reg;
                ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.size = STD_ARRAY_SIZE(read_reg);
                ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array[0].reg_addr = 0x00;
                rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
                        pCtxt->ctrl,
                        pSensor->serializer_alias,
                        &ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting);
                if (rc)
                {
                    SERR("Failed to read AR0820_EXT_ISP_GW5300 SER (0x%x) after remap", pSensor->serializer_alias);
                }
                else
                {
                    SHIGH("Detected AR0820_EXT_ISP_GW5300 SER alias 0x%x ",read_reg[0].reg_data);
                }
                if (pSensor->serializer_alias != ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array[0].reg_data)
                {
                    SENSOR_WARN("Remote SER address remap failed: 0x%x, should be 0x%x",
                        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array[0].reg_data, pSensor->serializer_alias);
                }
            }
        }
    }
    return rc;
}

static int ar0820_ext_isp_gw5300_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg)
{
    (void)ctxt;
    unsigned int mode;
    if (ctxt->max96712_sensors[link].mode < AR0820_EXT_ISP_GW5300_MODE_MAX)
    {
        mode = ctxt->max96712_sensors[link].mode;
    }
    else
    {
        SERR("The mode set is greater than tha MAX mode supported. Setting mode to Default.");
        mode = AR0820_EXT_ISP_GW5300_MODE_DEFAULT;
    }
    p_cfg->num_pipelines = 1;
    p_cfg->pipelines[0] = ar0820_ext_isp_gw5300_isp_pipelines[mode];

    return 0;
}

static int ar0820_ext_isp_gw5300_init_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0820_ext_isp_gw5300_contxt_t* ar0820_ext_isp_gw5300_ctxt = pSensor->pPrivCtxt;
    int rc = 0;

    SHIGH("ar0820_ext_isp_gw5300_init_link() E");
    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array = ar0820_ext_isp_gw5300_init_reg;
        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.size = STD_ARRAY_SIZE(ar0820_ext_isp_gw5300_init_reg);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pSensor->serializer_alias,
                    &ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting);
        if (rc)
        {
            SERR("Failed to init camera serializer(0x%x)", pSensor->serializer_alias);
            return -1;
        }
        pSensor->state = SENSOR_STATE_INITIALIZED;
        SHIGH("ar0820_ext_isp_gw5300 serilaizer init state %d", pSensor->state);
    }
    else
    {
        SERR("ar0820_ext_isp_gw5300 serilaizer at link %d init in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ar0820_ext_isp_gw5300_start_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0820_ext_isp_gw5300_contxt_t* ar0820_ext_isp_gw5300_ctxt = pSensor->pPrivCtxt;
    int rc = 0;

    SHIGH("ar0820_ext_isp_gw5300_start_link()");
    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SHIGH("starting serializer ");

        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array = ar0820_ext_isp_gw5300_start_reg;
        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.size = STD_ARRAY_SIZE(ar0820_ext_isp_gw5300_start_reg);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting);
    }
    else
    {
        SERR("serializer at link %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ar0820_ext_isp_gw5300_stop_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0820_ext_isp_gw5300_contxt_t* ar0820_ext_isp_gw5300_ctxt = pSensor->pPrivCtxt;
    int rc = 0;

    SHIGH("ar0820_ext_isp_gw5300_stop_link()");
    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.reg_array = ar0820_ext_isp_gw5300_stop_reg;
        ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting.size = STD_ARRAY_SIZE(ar0820_ext_isp_gw5300_stop_reg);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &ar0820_ext_isp_gw5300_ctxt->ar0820_ext_isp_gw5300_reg_setting)))
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }

        SLOW("serilaizer stop (0x%x)", pSensor->serializer_alias);
    }
    else
    {
        SERR("serializer at link %d stop in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}
