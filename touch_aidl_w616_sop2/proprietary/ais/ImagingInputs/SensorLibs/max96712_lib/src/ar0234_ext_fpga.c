/**
 * @file ar0234_ext_fpga.c
 *
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ar0234_ext_fpga.h"
#include <string.h>


#define AR0234_SENSOR_WIDTH                   1920
#define AR0234_SENSOR_HEIGHT                  1080

#define AR0234_CAM_SENSOR_DEFAULT_ADDR        0x20

#define MSM_SER_CHIP_ID_REG_ADDR              0xD
#define MSM_SER_REVISION_REG_ADDR             0XE
#define MSM_SER_REVISION_ES2                  0xFF

/*CONFIGURATION OPTIONS*/

#define _ar0234_ext_fpga_delay_               0
#define MAX9295_LINK_RESET_DELAY              100000
#define MAX9295_START_DELAY                   20000
#define MAX9295_STOP_DELAY                    20000

/**
 * For UYVY 8bpp set to QCARCAM_FMT_UYVY_8 / CSI_DT_RAW8
 * For UYVY 10bpp set to QCARCAM_FMT_UYVY_10 / CSI_DT_RAW10
 */


#define CID_VC0        0
#define CID_VC1        4



#define CAM_SER_INIT_AR0234 \
{ \
       { 0x0311, 0x10, _ar0234_ext_fpga_delay_}, \
       { 0x0308, 0x61, _ar0234_ext_fpga_delay_}, \
       { 0x0314, 0x22, _ar0234_ext_fpga_delay_}, \
       { 0x0316, 0x5e, _ar0234_ext_fpga_delay_}, \
       { 0x0318, 0x22, _ar0234_ext_fpga_delay_}, \
       { 0x031A, 0x22, _ar0234_ext_fpga_delay_}, \
       { 0x0002, 0x33, _ar0234_ext_fpga_delay_}, \
       { 0x02be, 0x90, _ar0234_ext_fpga_delay_}, \
       { 0x02bf, 0x60, _ar0234_ext_fpga_delay_}, \
       { 0x02ca, 0x80, _ar0234_ext_fpga_delay_}, \
       { 0x02cb, 0x60, _ar0234_ext_fpga_delay_}, \
       { 0x02d6, 0x80, _ar0234_ext_fpga_delay_}, \
       { 0x02d7, 0x60, _ar0234_ext_fpga_delay_}, \
       { 0x02d6, 0x80, _ar0234_ext_fpga_delay_}, \
       { 0x02d7, 0x60, _ar0234_ext_fpga_delay_}, \
       { 0x02d3, 0x80, _ar0234_ext_fpga_delay_}, \
       { 0x02d4, 0x60, _ar0234_ext_fpga_delay_}, \
       { 0x03F1, 0x00, _ar0234_ext_fpga_delay_}  \
}

#define CAM_SER_START \
{ \
    { 0x0002, 0x13, MAX9295_START_DELAY }, \
}

#define CAM_SER_STOP \
{ \
    { 0x0002, 0x03, MAX9295_STOP_DELAY }, \
}

// RESET_REGISTER - Start Streaming
#define AR0234_SENSOR_START_STREAM \
{ \
    { 0x301A, 0x205C, 200000 }, \
}

// RESET_REGISTER - Stop Streaming
#define AR0234_SENSOR_STOP_STREAM \
{ \
    { 0x301A, 0x2058, 200000 }, \
}



#define FLOAT_TO_FIXEDPOINT(b, f) \
  (((f)*(1<<(b))))

static int ar0234_ext_fpga_detect(max96712_context_t* ctxt, uint32 link);
static int ar0234_ext_fpga_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg);
static int ar0234_ext_fpga_init_link(max96712_context_t* ctxt, uint32 link);
static int ar0234_ext_fpga_start_link(max96712_context_t* ctxt, uint32 link);
static int ar0234_ext_fpga_stop_link(max96712_context_t* ctxt, uint32 link);

static int ar0234_ext_fpga_apply_exposure(max96712_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);
static int ar0234_ext_fpga_apply_gamma(max96712_context_t* ctxt, uint32 link, qcarcam_gamma_config_t* gamma);

static int ar0234_ext_fpga_initialize(max96712_context_t* ctxt, uint32 link, uint32 line_length);
static int ar0234_ext_fpga_sensor_start_stop(max96712_context_t* ctxt, uint32 link, uint32 start_stop);

max96712_sensor_t ar0234_ext_fpga_sensor_info = {
    .id = MAXIM_SENSOR_ID_AR0234_EXT_FPGA,
    .detect = ar0234_ext_fpga_detect,
    .get_link_cfg = ar0234_ext_fpga_get_link_cfg,

    .init_link = ar0234_ext_fpga_init_link,
    .start_link = ar0234_ext_fpga_start_link,
    .stop_link = ar0234_ext_fpga_stop_link,

    .apply_exposure = ar0234_ext_fpga_apply_exposure,
    .apply_gamma = ar0234_ext_fpga_apply_gamma
};

typedef struct
{
    struct camera_i2c_reg_setting cam_ser_reg_setting;
    struct camera_i2c_reg_setting ar0234_reg_setting;
}ar0234_ext_fpga_contxt_t;


static struct camera_i2c_reg_array max9295_init_8bit_regs[] = CAM_SER_INIT_AR0234;

static struct camera_i2c_reg_array max9295_start_reg[] = CAM_SER_START;
static struct camera_i2c_reg_array max9295_stop_reg[] = CAM_SER_STOP;

static struct camera_i2c_reg_array ar0234_start_stream_reg[] = AR0234_SENSOR_START_STREAM;
static struct camera_i2c_reg_array ar0234_stop_stream_reg[] = AR0234_SENSOR_STOP_STREAM;


static maxim_pipeline_t ar0234_ext_fpga_pipelines[AR0234_EXT_FPGA_MODE_MAX][MAXIM_LINK_MAX] =
{

    [AR0234_EXT_FPGA_MODE_YUV8_FULL_RES] = {
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = QCARCAM_FMT_UYVY_8,
                .res = {.width = AR0234_SENSOR_WIDTH, .height = AR0234_SENSOR_HEIGHT, .fps = 30.0f},
                .channel_info = {.vc = 0, .dt = CSI_DT_YUV422_8, .cid = CID_VC0},
            }
        }
    },
    [AR0234_EXT_FPGA_MODE_YUV8_FULL_RES_60FPS] = {
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = QCARCAM_FMT_UYVY_8,
                .res = {.width = AR0234_SENSOR_WIDTH, .height = AR0234_SENSOR_HEIGHT, .fps = 60.0f},
                .channel_info = {.vc = 0, .dt = CSI_DT_YUV422_8, .cid = CID_VC0},
            }
        }
    }
};

max96712_sensor_t* ar0234_ext_fpga_get_sensor_info(void)
{
    return &ar0234_ext_fpga_sensor_info;
}

static int ar0234_create_ctxt(max96712_sensor_info_t* pSensor)
{
    int rc = 0;

    if (pSensor->pPrivCtxt)
    {
        SLOW("Ctxt already created");
        return 0;
    }

    if(pSensor->mode >= AR0234_EXT_FPGA_MODE_MAX)
    {
        SERR("Unsupported sensor mode %d", pSensor->mode);
        return -1;
    }

    ar0234_ext_fpga_contxt_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(ar0234_ext_fpga_contxt_t));
    if (pCtxt)
    {
        memset(pCtxt, 0x0, sizeof(*pCtxt));

        pCtxt->cam_ser_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->cam_ser_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        pCtxt->ar0234_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->ar0234_reg_setting.data_type = CAMERA_I2C_WORD_DATA;

        pSensor->pPrivCtxt = pCtxt;
    }
    else
    {
        SERR("Failed to allocate sensor context");
        rc = -1;
    }

    return rc;
}

static int ar0234_ext_fpga_detect(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    int rc = 0;
    int i = 0;
    // List of serializer addresses we support. Last address is left blank for alias address
    uint16 supported_ser_addr[] = {0x80, 0x0};
    int num_addr = STD_ARRAY_SIZE(supported_ser_addr);
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    sensor_platform_func_table_t* sensor_fcn_tbl = &pCtxt->platform_fcn_tbl;
    ar0234_ext_fpga_contxt_t* ar0234_ctxt;

    rc = ar0234_create_ctxt(pSensor);
    if (rc)
    {
        SERR("Failed to create ctxt for link %d", link);
        return rc;
    }

    ar0234_ctxt = pSensor->pPrivCtxt;

    SHIGH("ar0234_ext_fpga_detect()");


    ar0234_ctxt->cam_ser_reg_setting.reg_array = read_reg;
    ar0234_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
    supported_ser_addr[num_addr-1] = pSensor->serializer_alias;

    /* Detect far end serializer */
    for (i = 0; i < num_addr; i++)
    {
        SHIGH("ar0234_ext_fpga_detect: read reg=MSM_SER_CHIP_ID_REG_ADDR(0xD) from slave=0x%x",supported_ser_addr[i]);

        ar0234_ctxt->cam_ser_reg_setting.reg_array[0].reg_addr = MSM_SER_CHIP_ID_REG_ADDR;

        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, supported_ser_addr[i], &ar0234_ctxt->cam_ser_reg_setting);
        if (!rc)
        {
            SHIGH("ar0234_ext_fpga_detect: successfully read reg=MSM_SER_CHIP_ID_REG_ADDR(0xD) from slave=0x%x; value=0x%x", 
                        supported_ser_addr[i], ar0234_ctxt->cam_ser_reg_setting.reg_array[0].reg_data);
                        pSensor->serializer_addr = supported_ser_addr[i];
            break;
        }
    }

    if (i == num_addr)
    {
        SENSOR_WARN("No Camera connected to Link %d of MAX96712 0x%x", link, pCtxt->slave_addr);
    }
    else if (pSensor->serializer_alias == pSensor->serializer_addr)
    {
        SENSOR_WARN("LINK %d already re-mapped", link);
        rc = 0;
    }
    else
    {
        struct camera_i2c_reg_array remap_ser[] = {
            {0x0, pSensor->serializer_alias, _ar0234_ext_fpga_delay_}
        };

        //link reset, remap cam, create broadcast addr,
        struct camera_i2c_reg_array remap_ser_2[] = {
            {0x0042, pSensor->sensor_alias, _ar0234_ext_fpga_delay_},
            {0x0043, AR0234_CAM_SENSOR_DEFAULT_ADDR, _ar0234_ext_fpga_delay_},
            {0x0044, CAM_SER_BROADCAST_ADDR, _ar0234_ext_fpga_delay_},
            {0x0045, pSensor->serializer_alias, _ar0234_ext_fpga_delay_}
        };

        SENSOR_WARN("Detected Camera connected to Link %d, ctxt=0x%x", link,  ctxt);

        ar0234_ctxt->cam_ser_reg_setting.reg_array = remap_ser;
        ar0234_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser);

        SHIGH("ar0234_ext_fpga_detect: write remap_ser (reg 0x0=0x%x) to slave=0x%x", pSensor->serializer_alias, pSensor->serializer_addr);

        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_addr, &ar0234_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to change serializer address (0x%x) of MAX96712 0x%x Link %d",
                pSensor->serializer_addr, pCtxt->slave_addr, link);
            return -1;
        }

        // Read mapped SER to double check if successful
        ar0234_ctxt->cam_ser_reg_setting.reg_array = read_reg;
        ar0234_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
        ar0234_ctxt->cam_ser_reg_setting.reg_array[0].reg_addr = 0x0000;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, pSensor->serializer_alias, &ar0234_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to read serializer(0x%x) after remap", pSensor->serializer_alias);
            return -1;
        }

        SHIGH("ar0234_ext_fpga_detect: read reg 0x0 from slave=0x%x; value=0x%x (it should equal 0x%x)",
                    pSensor->serializer_alias, ar0234_ctxt->cam_ser_reg_setting.reg_array[0].reg_data, pSensor->serializer_alias);

        if (pSensor->serializer_alias != ar0234_ctxt->cam_ser_reg_setting.reg_array[0].reg_data)
        {
            SENSOR_WARN("Remote SER address remap failed: 0x%x, should be 0x%x",
                ar0234_ctxt->cam_ser_reg_setting.reg_array[0].reg_data, pSensor->serializer_alias);
        }

        ar0234_ctxt->cam_ser_reg_setting.reg_array = remap_ser_2;
        ar0234_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser_2);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &ar0234_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            return -1;
        }

        SHIGH("ar0234_ext_fpga_detect: write remap_ser2 (link reset, remap cam, create broadcast addr) to slave=0x%x (serlzr alias)",
                        pSensor->serializer_alias);
    }

    return rc;
}

static int ar0234_ext_fpga_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg)
{
    int rc = 0;
    unsigned int mode;

    SHIGH("ar0234_ext_fpga_get_link_cfg(link=%d); return num_pipelines=1; ar0234_ext_fpga_pipelines[modes][link]",link);

    mode = ctxt->max96712_sensors[link].mode;

    if(mode < AR0234_EXT_FPGA_MODE_MAX)
    {
        p_cfg->num_pipelines = 1;
        p_cfg->pipelines[0] = ar0234_ext_fpga_pipelines[mode][0];
    }
    else
    {
        p_cfg->num_pipelines = 0;
        SERR("Invalid sensor mode %d", mode);
        rc = -1;
    }
    return rc;
}

static int ar0234_ext_fpga_init_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0234_ext_fpga_contxt_t* ar0234_ctxt = pSensor->pPrivCtxt;
    int rc=0, i;

    SHIGH("ar0234_ext_fpga_init_link()");
    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        uint32 line_length;
        unsigned int mode;

        mode = ctxt->max96712_sensors[link].mode;

        switch (ar0234_ext_fpga_pipelines[mode][0].mode.channel_info.dt)
        {
        case CSI_DT_RAW8:
        case CSI_DT_YUV422_8:
            ar0234_ctxt->cam_ser_reg_setting.reg_array = max9295_init_8bit_regs;
            ar0234_ctxt->cam_ser_reg_setting.size      = STD_ARRAY_SIZE(max9295_init_8bit_regs) ;
            break;
        default:
            SENSOR_WARN("link %d unknown DT: 0x%x", link,
                ar0234_ext_fpga_pipelines[mode][0].mode.channel_info.dt);
            rc = -1;
        }

        for (i = 0; i < ar0234_ctxt->cam_ser_reg_setting.size; i++) {
            SLOW("%d - 0x%x, 0x%x, %d", i, ar0234_ctxt->cam_ser_reg_setting.reg_array[i].reg_addr,
                                            ar0234_ctxt->cam_ser_reg_setting.reg_array[i].reg_data, 
                                            ar0234_ctxt->cam_ser_reg_setting.reg_array[i].delay);
        }

        if ( !rc ) 
        {
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                        pCtxt->ctrl,
                        pSensor->serializer_alias, &ar0234_ctxt->cam_ser_reg_setting);
            if (rc) {
                SERR("Failed to init camera serializer(0x%x)", pSensor->serializer_alias);
            }
        }

        if (!rc) 
        {
            SHIGH("Successfully wrote CAM_SER_INIT_AR0234 to camera serializer(0x%x)", pSensor->serializer_alias);
            pSensor->state = SENSOR_STATE_INITIALIZED;
        }

        if(ar0234_ext_fpga_pipelines[mode][0].mode.res.fps == 60.0 ) 
        {
             SHIGH("FPS set to 60 FPS");
            line_length = 0x03DA;       // 60 FPS
        }
        else {
            SHIGH("FPS set to 30 FPS");
            line_length = 0x07B4;       // 30 FPS
        }


        // write to AR0234 sensor
        ar0234_ext_fpga_initialize(ctxt, link, line_length);

        // bring FPGA out of reset
        struct camera_i2c_reg_array max9295_set_fpga_mode_and_reset_fpga[] = {
            {0x02D6, 0x90, 100000},
            {0x02D3, 0x80, 100000},
            {0x02D3, 0x90, 100000},
        };

        ar0234_ctxt->cam_ser_reg_setting.reg_array = max9295_set_fpga_mode_and_reset_fpga;
        ar0234_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_set_fpga_mode_and_reset_fpga);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &ar0234_ctxt->cam_ser_reg_setting);
        if (rc) 
        {
            SERR("failed to write max9295_set_fpga_mode_and_reset_fpga to max9295; slave_addr=0x%x", pSensor->serializer_alias);
        }
        else {
            SHIGH("successfully wrote max9295_set_fpga_mode_and_reset_fpga to max9295; slave_addr=0x%x", pSensor->serializer_alias);
        }

    }
    else {
        SERR("ar0234 %d init in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}


static int ar0234_ext_fpga_sensor_start_stop(max96712_context_t* ctxt, uint32 link, uint32 start_stop)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0234_ext_fpga_contxt_t* ar0234_ctxt = pSensor->pPrivCtxt;
    int rc = 0;

    if (start_stop)
    {
        ar0234_ctxt->ar0234_reg_setting.reg_array = ar0234_start_stream_reg;
        ar0234_ctxt->ar0234_reg_setting.size = STD_ARRAY_SIZE(ar0234_start_stream_reg);
    }
    else
    {
        ar0234_ctxt->ar0234_reg_setting.reg_array = ar0234_stop_stream_reg;
        ar0234_ctxt->ar0234_reg_setting.size = STD_ARRAY_SIZE(ar0234_stop_stream_reg);
    }

    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pSensor->sensor_alias, &ar0234_ctxt->ar0234_reg_setting);
    if (rc)
    {
        SERR("failed to write ar234_ss to ar234; slave_addr=0x%x",pSensor->sensor_alias);
    }
    else {
        SHIGH("successfully wrote ar234_ss to ar234; slave_addr=0x%x",pSensor->sensor_alias);
    }

    return rc;
}

#define DEFAULT_GAIN 46

static int ar0234_ext_fpga_initialize(max96712_context_t* ctxt, uint32 link, uint32 line_length)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0234_ext_fpga_contxt_t* ar0234_ctxt = pSensor->pPrivCtxt;
    int rc=0;

    struct camera_i2c_reg_array ar0234_wr_array[] = {
           {0x301A, 0x00D9, 200},
           {0x31E0, 0x0003, _ar0234_ext_fpga_delay_},
           {0x31E0, 0x0003, _ar0234_ext_fpga_delay_},
           {0x30B0, 0x0028, _ar0234_ext_fpga_delay_},
           {0x3060, DEFAULT_GAIN, 200},
           {0x3088, 0x8000, _ar0234_ext_fpga_delay_},
           {0x3086, 0xC1AE, _ar0234_ext_fpga_delay_},
           {0x3086, 0x327F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5780, _ar0234_ext_fpga_delay_},
           {0x3086, 0x272F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7416, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7E13, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8000, _ar0234_ext_fpga_delay_},
           {0x3086, 0x307E, _ar0234_ext_fpga_delay_},
           {0x3086, 0xFF80, _ar0234_ext_fpga_delay_},
           {0x3086, 0x20C3, _ar0234_ext_fpga_delay_},
           {0x3086, 0xB00E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8190, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1643, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1651, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9D3E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9545, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2209, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3781, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9016, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4316, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F90, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8000, _ar0234_ext_fpga_delay_},
           {0x3086, 0x387F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1380, _ar0234_ext_fpga_delay_},
           {0x3086, 0x233B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F93, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4502, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8000, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7FB0, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8D66, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F90, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8192, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3C16, _ar0234_ext_fpga_delay_},
           {0x3086, 0x357F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9345, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0280, _ar0234_ext_fpga_delay_},
           {0x3086, 0x007F, _ar0234_ext_fpga_delay_},
           {0x3086, 0xB08D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x667F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9081, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8237, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4502, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3681, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8044, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1631, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4374, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1678, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7B7D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4502, _ar0234_ext_fpga_delay_},
           {0x3086, 0x450A, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7E12, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8180, _ar0234_ext_fpga_delay_},
           {0x3086, 0x377F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1045, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0A0E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7FD4, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8024, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0E82, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9CC2, _ar0234_ext_fpga_delay_},
           {0x3086, 0xAFA8, _ar0234_ext_fpga_delay_},
           {0x3086, 0xAA03, _ar0234_ext_fpga_delay_},
           {0x3086, 0x430D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2D46, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4316, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5F16, _ar0234_ext_fpga_delay_},
           {0x3086, 0x530D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1660, _ar0234_ext_fpga_delay_},
           {0x3086, 0x401E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2904, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2984, _ar0234_ext_fpga_delay_},
           {0x3086, 0x81E7, _ar0234_ext_fpga_delay_},
           {0x3086, 0x816F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1706, _ar0234_ext_fpga_delay_},
           {0x3086, 0x81E7, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F81, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5C0D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5754, _ar0234_ext_fpga_delay_},
           {0x3086, 0x495F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5305, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5307, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4D2B, _ar0234_ext_fpga_delay_},
           {0x3086, 0xF810, _ar0234_ext_fpga_delay_},
           {0x3086, 0x164C, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0755, _ar0234_ext_fpga_delay_},
           {0x3086, 0x562B, _ar0234_ext_fpga_delay_},
           {0x3086, 0xB82B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x984E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1129, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9460, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5C09, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5C1B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4002, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4500, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4580, _ar0234_ext_fpga_delay_},
           {0x3086, 0x29B6, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F80, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4004, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F88, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4109, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5C0B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x29B2, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4125, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5C03, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4105, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5F2B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x902B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8081, _ar0234_ext_fpga_delay_},
           {0x3086, 0x6F40, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1041, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0160, _ar0234_ext_fpga_delay_},
           {0x3086, 0x29A2, _ar0234_ext_fpga_delay_},
           {0x3086, 0x29A3, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5F4D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1C17, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0281, _ar0234_ext_fpga_delay_},
           {0x3086, 0xE729, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8345, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8840, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0B7F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8A40, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2345, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8024, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4008, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F88, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5D29, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9288, _ar0234_ext_fpga_delay_},
           {0x3086, 0x102B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0489, _ar0234_ext_fpga_delay_},
           {0x3086, 0x165C, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4386, _ar0234_ext_fpga_delay_},
           {0x3086, 0x170B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5C03, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8A48, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4D4E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2B80, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4C09, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4119, _ar0234_ext_fpga_delay_},
           {0x3086, 0x816F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4110, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4001, _ar0234_ext_fpga_delay_},
           {0x3086, 0x6029, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8229, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8329, _ar0234_ext_fpga_delay_},
           {0x3086, 0x435C, _ar0234_ext_fpga_delay_},
           {0x3086, 0x055F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4D1C, _ar0234_ext_fpga_delay_},
           {0x3086, 0x81E7, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4502, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8180, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F80, _ar0234_ext_fpga_delay_},
           {0x3086, 0x410A, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9144, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1609, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2FC3, _ar0234_ext_fpga_delay_},
           {0x3086, 0xB130, _ar0234_ext_fpga_delay_},
           {0x3086, 0xC3B1, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0343, _ar0234_ext_fpga_delay_},
           {0x3086, 0x164A, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0A43, _ar0234_ext_fpga_delay_},
           {0x3086, 0x160B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4316, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8F43, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1690, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4316, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F81, _ar0234_ext_fpga_delay_},
           {0x3086, 0x450A, _ar0234_ext_fpga_delay_},
           {0x3086, 0x410F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7F83, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5D29, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4488, _ar0234_ext_fpga_delay_},
           {0x3086, 0x102B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0453, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0D40, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1045, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0240, _ar0234_ext_fpga_delay_},
           {0x3086, 0x087F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8053, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0D89, _ar0234_ext_fpga_delay_},
           {0x3086, 0x165C, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4586, _ar0234_ext_fpga_delay_},
           {0x3086, 0x170B, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5C05, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8A60, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4B91, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4416, _ar0234_ext_fpga_delay_},
           {0x3086, 0x09C1, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2CA9, _ar0234_ext_fpga_delay_},
           {0x3086, 0xAB30, _ar0234_ext_fpga_delay_},
           {0x3086, 0x51B3, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3D5A, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7E3D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7E19, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8000, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8B1F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2A1F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x83A2, _ar0234_ext_fpga_delay_},
           {0x3086, 0x153D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x6F7E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1175, _ar0234_ext_fpga_delay_},
           {0x3086, 0x16AD, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3345, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0A7F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x5380, _ar0234_ext_fpga_delay_},
           {0x3086, 0x238C, _ar0234_ext_fpga_delay_},
           {0x3086, 0x667F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1381, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8414, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8180, _ar0234_ext_fpga_delay_},
           {0x3086, 0x313D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x6445, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2A94, _ar0234_ext_fpga_delay_},
           {0x3086, 0x519E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x963D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2B3D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x1B52, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9F0E, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3D08, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3D16, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7E3D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7E19, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8024, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8B6F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2A6F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x83A2, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7E11, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7516, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3E97, _ar0234_ext_fpga_delay_},
           {0x3086, 0x0E82, _ar0234_ext_fpga_delay_},
           {0x3086, 0xB23D, _ar0234_ext_fpga_delay_},
           {0x3086, 0x7FAC, _ar0234_ext_fpga_delay_},
           {0x3086, 0x3E45, _ar0234_ext_fpga_delay_},
           {0x3086, 0x027F, _ar0234_ext_fpga_delay_},
           {0x3086, 0xD080, _ar0234_ext_fpga_delay_},
           {0x3086, 0x008C, _ar0234_ext_fpga_delay_},
           {0x3086, 0x667F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x9081, _ar0234_ext_fpga_delay_},
           {0x3086, 0x943F, _ar0234_ext_fpga_delay_},
           {0x3086, 0x4416, _ar0234_ext_fpga_delay_},
           {0x3086, 0x8184, _ar0234_ext_fpga_delay_},
           {0x3086, 0x162C, _ar0234_ext_fpga_delay_},
           {0x3086, 0x2C2C, _ar0234_ext_fpga_delay_},
           {0x31AE, 0x0204, _ar0234_ext_fpga_delay_},
           {0x3030, 0x0026, _ar0234_ext_fpga_delay_},
           {0x302E, 0x0003, _ar0234_ext_fpga_delay_},
           {0x302C, 0x0001, _ar0234_ext_fpga_delay_},
           {0x302A, 0x0005, _ar0234_ext_fpga_delay_},
           {0x3038, 0x0001, _ar0234_ext_fpga_delay_},
           {0x3036, 0x000A, _ar0234_ext_fpga_delay_},
           {0x31B0, 0x0082, _ar0234_ext_fpga_delay_},
           {0x31B2, 0x005C, _ar0234_ext_fpga_delay_},
           {0x31B4, 0x3165, _ar0234_ext_fpga_delay_},
           {0x31B6, 0x310E, _ar0234_ext_fpga_delay_},
           {0x31B8, 0x2047, _ar0234_ext_fpga_delay_},
           {0x31BA, 0x0305, _ar0234_ext_fpga_delay_},
           {0x31BC, 0x8004, _ar0234_ext_fpga_delay_},
           {0x31D0, 0x0000, _ar0234_ext_fpga_delay_},
           {0x3002, 0x0008, _ar0234_ext_fpga_delay_},
           {0x3004, 0x0008, _ar0234_ext_fpga_delay_},
           {0x3006, 0x04B7, _ar0234_ext_fpga_delay_},
           {0x3008, 0x0787, _ar0234_ext_fpga_delay_},
           {0x3064, 0x1802, _ar0234_ext_fpga_delay_},
           {0x300A, 0x04c4, _ar0234_ext_fpga_delay_},
           {0x300C, 0x0aA0, _ar0234_ext_fpga_delay_},
           {0x30A2, 0x0001, _ar0234_ext_fpga_delay_},
           {0x30A6, 0x0001, _ar0234_ext_fpga_delay_},
           {0x3012, 0x02DC, _ar0234_ext_fpga_delay_},
           {0x3786, 0x0006, _ar0234_ext_fpga_delay_},
           {0x3044, 0x0410, _ar0234_ext_fpga_delay_},
           {0x3094, 0x03D4, _ar0234_ext_fpga_delay_},
           {0x3096, 0x0480, _ar0234_ext_fpga_delay_},
           {0x30B0, 0x0028, _ar0234_ext_fpga_delay_},
           {0x30BA, 0x7602, _ar0234_ext_fpga_delay_},
           {0x30FE, 0x002A, _ar0234_ext_fpga_delay_},
           {0x31DE, 0x0410, _ar0234_ext_fpga_delay_},
           {0x3ED6, 0x1435, _ar0234_ext_fpga_delay_},
           {0x3ED8, 0x9865, _ar0234_ext_fpga_delay_},
           {0x3EDA, 0x7698, _ar0234_ext_fpga_delay_},
           {0x3EDC, 0x99FF, _ar0234_ext_fpga_delay_},
           {0x3EE2, 0xBB88, _ar0234_ext_fpga_delay_},
           {0x3EE4, 0x8836, _ar0234_ext_fpga_delay_},
           {0x3EF0, 0x1CF0, _ar0234_ext_fpga_delay_},
           {0x3EF2, 0x0000, _ar0234_ext_fpga_delay_},
           {0x3EF8, 0x6166, _ar0234_ext_fpga_delay_},
           {0x3EFA, 0x3333, _ar0234_ext_fpga_delay_},
           {0x3EFC, 0x6634, _ar0234_ext_fpga_delay_},
           {0x3276, 0x05DC, _ar0234_ext_fpga_delay_},
           {0x3ECC, 0x6E42, _ar0234_ext_fpga_delay_},
           {0x3ECC, 0x0E42, _ar0234_ext_fpga_delay_},
           {0x3EEC, 0x0C0C, _ar0234_ext_fpga_delay_},
           {0x3EE8, 0xAAE4, _ar0234_ext_fpga_delay_},
           {0x3EE6, 0x3363, _ar0234_ext_fpga_delay_},
           {0x3EE6, 0x3363, _ar0234_ext_fpga_delay_},
           {0x3EE8, 0xAAE4, _ar0234_ext_fpga_delay_},
           {0x3EE8, 0xAAE4, _ar0234_ext_fpga_delay_},
           {0x3060, DEFAULT_GAIN, _ar0234_ext_fpga_delay_},
           {0x301A, 0x2058, _ar0234_ext_fpga_delay_},
           {0x31DC, 0x0034, _ar0234_ext_fpga_delay_},
           {0x300A, 0x0480, _ar0234_ext_fpga_delay_},
           {0x300C, line_length, _ar0234_ext_fpga_delay_}, // LINE_LENGTH_PCK  60FPS : 0x03da , 30FPS : 0x07B4
           {0x3012,    120, _ar0234_ext_fpga_delay_},
           {0x3064, 0x1882, _ar0234_ext_fpga_delay_},
           {0x3064, 0x1982, _ar0234_ext_fpga_delay_},
           {0x3100, 0x0001, _ar0234_ext_fpga_delay_},
           {0x3102, 0x8000, _ar0234_ext_fpga_delay_},
           {0x311c,    120, _ar0234_ext_fpga_delay_},
           {0x311e,      2, _ar0234_ext_fpga_delay_},
           {0x3ED2, 0xAA86, _ar0234_ext_fpga_delay_},
           {0x3EEE, 0xA4AA, _ar0234_ext_fpga_delay_},
           {0x301D, 0x0300, _ar0234_ext_fpga_delay_}, // VERTICAL FLIP
           {0x3040, 0xc000, _ar0234_ext_fpga_delay_},
           {0x3270, 0x0100, _ar0234_ext_fpga_delay_}, // enable flash output
           {0x3006, 0x043F, _ar0234_ext_fpga_delay_},
           {0x3008, 0x0787, _ar0234_ext_fpga_delay_},
        };

        ar0234_ctxt->ar0234_reg_setting.reg_array = ar0234_wr_array;
        ar0234_ctxt->ar0234_reg_setting.size = STD_ARRAY_SIZE(ar0234_wr_array);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pSensor->sensor_alias, &ar0234_ctxt->ar0234_reg_setting);
        if (rc)
        {
            SERR("failed to initialize ar234; slave_addr=0x%x",pSensor->sensor_alias);
        }
        else {
            SHIGH("ar0234(0x%x) init success",pSensor->sensor_alias);
        }

        return 0;
}

static int ar0234_ext_fpga_start_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0234_ext_fpga_contxt_t* ar0234_ctxt = pSensor->pPrivCtxt;
    int rc = 0, i=0;

    SHIGH("ar0234_ext_fpga_start_link() link=%d",link);

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {

        SHIGH("**** starting Sensor **** ");
        ar0234_ext_fpga_sensor_start_stop(ctxt, link, 1);

        SHIGH("**** starting serializer **** ");

        ar0234_ctxt->cam_ser_reg_setting.reg_array = max9295_start_reg;
        ar0234_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_start_reg);

        SHIGH("ar0234_ext_fpga_start_link; *BYPASS* write CAM_SER_START to slave=0x%x", pSensor->serializer_alias);

        for (i = 0; i < ar0234_ctxt->cam_ser_reg_setting.size; i++)
        {
            SLOW("%d - 0x%x, 0x%x, %d", i, ar0234_ctxt->cam_ser_reg_setting.reg_array[i].reg_addr,
                                           ar0234_ctxt->cam_ser_reg_setting.reg_array[i].reg_data,
                                           ar0234_ctxt->cam_ser_reg_setting.reg_array[i].delay);
        }

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &ar0234_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("serializer 0x%x failed to start", pSensor->serializer_alias);

            SHIGH("**** stoping Sensor **** ");
            ar0234_ext_fpga_sensor_start_stop(ctxt, link, 0);
        }
        else
        {
            SHIGH("serializer 0x%x state set to SENSOR_STATE_STREAMING", pSensor->serializer_alias);
            pSensor->state = SENSOR_STATE_STREAMING;
        }
    }
    else
    {
        SERR("ar0234 %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ar0234_ext_fpga_stop_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0234_ext_fpga_contxt_t* ar0234_ctxt = pSensor->pPrivCtxt;
    int rc;

    SHIGH("ar0234_ext_fpga_stop_link() link=%d",link);

    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        SHIGH("ar0234_ext_fpga_start_link; write CAM_SER_STOP to slave=0x%x",pSensor->serializer_alias);

        ar0234_ctxt->cam_ser_reg_setting.reg_array = max9295_stop_reg;
        ar0234_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_stop_reg);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &ar0234_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }

        SHIGH("**** stoping Sensor **** ");
        ar0234_ext_fpga_sensor_start_stop(ctxt, link, 0);

        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("ar0234 %d stop in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}



static int ar0234_ext_fpga_apply_exposure(max96712_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    int rc = 0;

    SHIGH("ar0234_ext_fpga_apply_exposure()");

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SERR("ar0234_ext_fpga %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }
    else if(exposure_info->exposure_mode_type == QCARCAM_EXPOSURE_AUTO)
    {
        SENSOR_WARN("ar0234_ext_fpga_apply_exposure: QCARCAM_EXPOSURE_AUTO requested");

    }
    else if (exposure_info->exposure_mode_type == QCARCAM_EXPOSURE_MANUAL)
    {
        SENSOR_WARN("ar0234_ext_fpga_apply_exposure: QCARCAM_EXPOSURE_MANUAL requested");

    }
    else
    {
        //TODO: pass-through for now
        SERR("ALIar0234_ext_fpga: not implemented AEC mode %d", exposure_info->exposure_mode_type);
        rc = 0;
    }

    return rc;
}

static int ar0234_ext_fpga_apply_gamma(max96712_context_t* ctxt, uint32 link, qcarcam_gamma_config_t* gamma_info)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    int rc = 0;

    SHIGH("ar0234_ext_fpga_apply_gamma()");

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SERR("ar0234_ext_fpga %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }
    else if(gamma_info->config_type == QCARCAM_GAMMA_EXPONENT)
    {
        //unsigned int reg_value = gamma_info->gamma.f_value * 100;
        SENSOR_WARN("ar0234_ext_fpga_apply_gamma: QCARCAM_GAMMA_EXPONENT selected");
    }
    else if (gamma_info->config_type == QCARCAM_GAMMA_KNEEPOINTS)
    {
        //unsigned int table_len = gamma_info->gamma.table.length;
       // unsigned int *p_value = gamma_info->gamma.table.p_value;
       // unsigned int idx = 0;

        SENSOR_WARN("ar0234_ext_fpga_apply_gamma: QCARCAM_GAMMA_KNEEPOINTS selected");

    }
    else
    {
        SERR("unsupported gamma configure type for ar0234_ext_fpga 0x%x", pSensor->sensor_alias);
        return -1;
    }

    return rc;
}
