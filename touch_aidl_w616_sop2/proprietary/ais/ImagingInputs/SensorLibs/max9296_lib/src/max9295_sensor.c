/**
 * @file max9295_sensor.c
 *
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "max9295_sensor.h"

static int ser9295_detect(max9296_context_t* ctxt, uint32 link);
static int ser9295_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg);
static int ser9295_init_link(max9296_context_t* ctxt, uint32 link);
static int ser9295_start_link(max9296_context_t* ctxt, uint32 link);
static int ser9295_stop_link(max9296_context_t* ctxt, uint32 link);

/*Custom functions operate on serilizer */
static max9296_sensor_t ser9295_info = {
    .id = MAXIM_SENSOR_ID_MAX9295,
    .detect = ser9295_detect,
    .get_link_cfg = ser9295_get_link_cfg,

    .init_link = ser9295_init_link,
    .start_link = ser9295_start_link,
    .stop_link = ser9295_stop_link,
};


static struct camera_i2c_reg_setting cam_ser_reg_setting =
{
    .reg_array = NULL,
    .size = 0,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
};

static struct camera_i2c_reg_array max9295_gmsl_0[] = CAM_SER9295_ADDR_CHNG_A;
static struct camera_i2c_reg_array max9295_gmsl_1[] = CAM_SER9295_ADDR_CHNG_B;
static struct camera_i2c_reg_array max9295_init_0[] = CAM_SER9295_INIT_A;
static struct camera_i2c_reg_array max9295_init_1[] = CAM_SER9295_INIT_B;
static struct camera_i2c_reg_array max9295_start_reg[] = CAM_SER9295_START;
static struct camera_i2c_reg_array max9295_stop_reg[] = CAM_SER9295_STOP;

// List of serializer addresses we support
static uint16 supported_ser_addr[] = {0xC4, 0x88, 0x80, 0x70, 0x74, 0};

static maxim_pipeline_t ser9295_isp_pipelines[MAX9295_MODE_MAX][MAXIM_LINK_MAX] =
{
    [MAX9295_MODE_RECORDER_YUV8_FULL_RES] = {
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = QCARCAM_FMT_UYVY_8,
                .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                .channel_info = {.vc = 0, .dt = CSI_DT_RAW8, .cid = CID_VC0},
            }
        },
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = QCARCAM_FMT_UYVY_8,
                .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                .channel_info = {.vc = 1, .dt = CSI_DT_RAW8, .cid = CID_VC1},
            }
        }
    }
};

max9296_sensor_t* ser9295_get_info(void)
{
    return &ser9295_info;
}


/**
*******************************************************************************
 * FUNCTION:     ser9295_detect
 *
 * DESCRIPTION:  Detect far end serializer
*******************************************************************************
**/
static int ser9295_detect(max9296_context_t* pCtxt_in, uint32 link)
{
    SHIGH("ser9295_detect()");
    int rc = 0;
    if(NULL == pCtxt_in)
    {
        SERR("Invalid pCtxt_in pointer (0x%x)",pCtxt_in);
        goto EXIT;
    }
    
    max9296_context_t* pCtxt = (max9296_context_t*)pCtxt_in;
    int i = 0;
    int num_addr = STD_ARRAY_SIZE(supported_ser_addr);
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];
    sensor_platform_func_table_t* sensor_fcn_tbl = &pCtxt->platform_fcn_tbl;
    
    cam_ser_reg_setting.reg_array = read_reg;
    cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
    supported_ser_addr[num_addr-1] = pSensor->serializer_alias;

    /* Detect far end serializer */
    for (i = 0; i < num_addr; i++)
    {
        cam_ser_reg_setting.reg_array[0].reg_addr = MSM_SER_CHIP_ID_REG_ADDR;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, supported_ser_addr[i], &cam_ser_reg_setting);
        if (!rc)
        {
            pSensor->serializer_addr = supported_ser_addr[i];
            break;
        }
    }

    if (i == num_addr)
    {
        SENSOR_WARN("No Camera connected to Link %d of MAX9296 0x%x or ser addr 0x%x", link, pCtxt->slave_addr, pSensor->serializer_addr);
    }
    else if (pSensor->serializer_alias == pSensor->serializer_addr)
    {
        SENSOR_WARN("LINK %d already re-mapped with ser addr 0x%x", link, pSensor->serializer_addr);
        rc = 0;
    }
    else
    {
        struct camera_i2c_reg_array remap_ser[] = {
            {0x0, pSensor->serializer_alias, _max9295_delay_}
        };

        //link reset, remap cam, create broadcast addr,
        struct camera_i2c_reg_array remap_ser_2[] = {
            {0x0010, 0x31, MAX9295_LINK_RESET_DELAY },
            {0x0042, pSensor->sensor_alias, _max9295_delay_},
            {0x0043, CAM_EXT_ISP_DEFAULT_ADDR, _max9295_delay_},
            {0x0044, CAM_SER_BROADCAST_ADDR, _max9295_delay_},
            {0x0045, pSensor->serializer_alias, _max9295_delay_}
        };

        SENSOR_WARN("Detected Camera connected to Link %d, Ser ID[0x%x]: 0x%x",
            link, MSM_SER_CHIP_ID_REG_ADDR, cam_ser_reg_setting.reg_array[0].reg_data);

        cam_ser_reg_setting.reg_array = remap_ser;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl,pSensor->serializer_addr, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to change serializer address (0x%x) of MAX9296 0x%x Link %d",
                pSensor->serializer_addr, pCtxt->slave_addr, link);
            goto EXIT;
        }

        cam_ser_reg_setting.reg_array = remap_ser_2;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser_2);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            goto EXIT;
        }

        cam_ser_reg_setting.reg_array = link ? max9295_gmsl_1 : max9295_gmsl_0;
        cam_ser_reg_setting.size = link ? STD_ARRAY_SIZE(max9295_gmsl_1) : STD_ARRAY_SIZE(max9295_gmsl_0);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            goto EXIT;
        }

        // Read mapped SER to double check if successful
        cam_ser_reg_setting.reg_array = read_reg;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
        cam_ser_reg_setting.reg_array[0].reg_addr = 0x0000;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, pSensor->serializer_alias, &cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to read serializer(0x%x) after remap", pSensor->serializer_alias);
            goto EXIT;
        }

        if (pSensor->serializer_alias != cam_ser_reg_setting.reg_array[0].reg_data)
        {
            SENSOR_WARN("Remote SER address remap failed: 0x%x, should be 0x%x",
                cam_ser_reg_setting.reg_array[0].reg_data, pSensor->serializer_alias);
        }
    }
EXIT:
    return rc;
}

/**
 * FUNCTION: ser9295_get_link_cfg
 *
 * DESCRIPTION: Gets the settings for each serializer pipeline
 **/
static int ser9295_get_link_cfg(max9296_context_t* ctxt, uint32 link, max9296_link_cfg_t* p_cfg)
{
    int rc = 0;
    if(NULL == ctxt)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        goto EXIT;
    }
    unsigned int mode;   
    mode = ctxt->max9296_sensors[link].mode;
    if(mode < MAX9295_MODE_MAX)
    {
        p_cfg->num_pipelines = 1;
        p_cfg->pipelines[0] = ser9295_isp_pipelines[mode][link];
    }
    else
    {
        p_cfg->num_pipelines = 0;
        SERR("Invalid sensor mode %d", mode);
        rc = CAMERA_EBADPARM;
    }
EXIT:
    return rc;
}


/**
 * FUNCTION: ser9295_init_link
 *
 * DESCRIPTION: Initialization of ser9295 camera module is performed here
 **/
static int ser9295_init_link(max9296_context_t* ctxt, uint32 link)
{
    SHIGH("ser9295_init_link()");
    int rc = 0;
    if(NULL == ctxt)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        goto EXIT;
    }
    
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];

    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        unsigned int mode;
        mode = ctxt->max9296_sensors[link].mode;
        switch (ser9295_isp_pipelines[mode][link].mode.channel_info.dt)
        {
            case CSI_DT_RAW8:
                cam_ser_reg_setting.reg_array = link ? max9295_init_1 : max9295_init_0;
                cam_ser_reg_setting.size = link ? STD_ARRAY_SIZE(max9295_init_1) : STD_ARRAY_SIZE(max9295_init_0);
                break;
            default:
                SENSOR_WARN("link %d unknown DT: 0x%x", link,
                    ser9295_isp_pipelines[mode][link].mode.channel_info.dt);
                rc = CAMERA_EBADITEM;
                break;
        }

        if (!rc)
        {
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pSensor->serializer_alias, &cam_ser_reg_setting);
            if (rc)
            {
                SERR("Failed to init camera serializer(0x%x)", pSensor->serializer_alias);
            }
        }

        if(!rc)
        {
            pSensor->state = SENSOR_STATE_INITIALIZED;
        }
    }
    else
    {
        SERR("ser9295 %d init in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }
EXIT:
    return rc;
}

/**
 * FUNCTION: ser9295_start_link
 *
 * DESCRIPTION: Custom function which is responsible for start sensor streaming
 **/
static int ser9295_start_link(max9296_context_t* ctxt, uint32 link)
{
    SHIGH("ser9295_start_link()");
    int rc = 0;
    if(NULL == ctxt)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        goto EXIT;
    }
    
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SHIGH("starting serializer for Video Recorder mode.");
        cam_ser_reg_setting.reg_array = max9295_start_reg;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_start_reg);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &cam_ser_reg_setting);
        if (rc)
        {
            SERR("serializer 0x%x failed to start", pSensor->serializer_alias);
        }
        else
        {
            pSensor->state = SENSOR_STATE_STREAMING;
        }
    }
    else
    {
        SERR("ser9295 %d start in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }
EXIT:
    return rc;
}

/**
 * FUNCTION: ser9295_stop_link
 *
 * DESCRIPTION: Custom function which is responsible for stop sensor streaming
 **/
static int ser9295_stop_link(max9296_context_t* ctxt, uint32 link)
{
    SHIGH("ser9295_stop_link()");
    int rc = 0;
    if(NULL == ctxt)
    {
        SERR("Invalid ctxt pointer (0x%x)",ctxt);
        goto EXIT;
    }
    
    max9296_context_t* pCtxt = ctxt;
    max9296_sensor_info_t* pSensor = &pCtxt->max9296_sensors[link];

    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        cam_ser_reg_setting.reg_array = max9295_stop_reg;
        cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_stop_reg);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &cam_ser_reg_setting)))
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }

        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("ser9295 %d stop in wrong state %d", link, pSensor->state);
        rc = CAMERA_EBADSTATE;
    }
EXIT:
    return rc;
}
