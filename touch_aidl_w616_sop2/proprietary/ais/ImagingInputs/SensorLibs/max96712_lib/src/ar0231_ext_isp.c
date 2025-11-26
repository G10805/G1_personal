/**
 * @file ar0231_ext_isp.c
 *
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ar0231_ext_isp.h"
#include <string.h>

#define SENSOR_WIDTH    1920
#define SENSOR_HEIGHT   1024

// The sensor embeds extra 4 lines of metadata with ISP parameters
//crop the top and bottom 2 lines by default
#define SENSOR_HEIGHT_CROP_META   1020
#define SENSOR_X_OFFSET   0
#define SENSOR_Y_OFFSET   2


#define CAM_SENSOR_DEFAULT_ADDR         0xE0
#define CAM_EXT_ISP_DEFAULT_ADDR        0xBA

#define MSM_SER_CHIP_ID_REG_ADDR              0xD
#define MSM_SER_REVISION_REG_ADDR             0XE
#define MSM_SER_REVISION_ES2                  0xFF

#define CAM_EXT_ISP_CHIP_ID    0x64

/*CONFIGURATION OPTIONS*/

#define _ar0231_ext_isp_delay_ 0
#define MAX9295_LINK_RESET_DELAY 100000
#define MAX9295_START_DELAY 20000
#define MAX9295_STOP_DELAY 20000

#define FMT_UYVY_MIPI10 QCARCAM_COLOR_FMT(QCARCAM_YUV_UYVY, QCARCAM_BITDEPTH_10, QCARCAM_PACK_MIPI)

#define CID_VC0        0
#define CID_VC1        4


/*
Reg 1B0 - 1B9
Use ‘crossbar’ in the serializer move bits from 10 to 8, moving last two LSB to MSB

9 .... 0 -> 1098765432 in serialzer (cross bar)
10 | 98765432 (8 bits) in deser (truncate)

Deseralizer is programmed to truncate from 10 to 8 (in MSB). Effective, you get 9 - 2, truncating just the 2 LSB bits
*/
#define CAM_SER_INIT_8BIT \
{ \
    { 0x0002, 0x03, _ar0231_ext_isp_delay_ }, \
    { 0x0100, 0x60, _ar0231_ext_isp_delay_ }, \
    { 0x0101, 0x4A, _ar0231_ext_isp_delay_ }, \
    { 0x01B0, 0x02, _ar0231_ext_isp_delay_ }, \
    { 0x01B1, 0x03, _ar0231_ext_isp_delay_ }, \
    { 0x01B2, 0x04, _ar0231_ext_isp_delay_ }, \
    { 0x01B3, 0x05, _ar0231_ext_isp_delay_ }, \
    { 0x01B4, 0x06, _ar0231_ext_isp_delay_ }, \
    { 0x01B5, 0x07, _ar0231_ext_isp_delay_ }, \
    { 0x01B6, 0x08, _ar0231_ext_isp_delay_ }, \
    { 0x01B7, 0x09, _ar0231_ext_isp_delay_ }, \
    { 0x01B8, 0x00, _ar0231_ext_isp_delay_ }, \
    { 0x01B9, 0x01, _ar0231_ext_isp_delay_ }, \
    { 0x0007, 0x07, _ar0231_ext_isp_delay_ }, \
}

#define CAM_SER_INIT_10BIT \
{ \
    { 0x0002, 0x03, _ar0231_ext_isp_delay_ }, \
    { 0x0100, 0x60, _ar0231_ext_isp_delay_ }, \
    { 0x0101, 0x4A, _ar0231_ext_isp_delay_ }, \
    { 0x0007, 0x07, _ar0231_ext_isp_delay_ }, \
}

#define CAM_SER_START \
{ \
    { 0x0002, 0x13, MAX9295_START_DELAY }, \
}

#define CAM_SER_STOP \
{ \
    { 0x0002, 0x03, MAX9295_STOP_DELAY }, \
}

#define AP0200_SYS_CMD \
{ \
    { 0x0040, 0x0000, _ar0231_ext_isp_delay_}, \
}

#define AP0200_TRIGGER_MODE \
{ \
    { 0xC890, 0x03, _ar0231_ext_isp_delay_}, \
    { 0xC891, 0x03, _ar0231_ext_isp_delay_}, \
    { 0xC892, 0x00, _ar0231_ext_isp_delay_}, \
    { 0xCB59, 0x00, _ar0231_ext_isp_delay_}, \
}

#define AP0200_CHANGE_CONFIG \
{ \
    { 0xFC00, 0x2800, _ar0231_ext_isp_delay_}, \
    { 0x0040, 0x8100, _ar0231_ext_isp_delay_}, \
}

#define AP0200_AE_MODE_AUTO \
{ \
    { 0xC8BC, 0x04, _ar0231_ext_isp_delay_}, \
}

#define AP0200_AE_MODE_MANUAL \
{ \
    { 0xC8BC, 0x24, _ar0231_ext_isp_delay_}, \
}

#define AP0200_EXP_CONFIG \
{ \
    { 0xC8C0, 0x0500, _ar0231_ext_isp_delay_}, \
    { 0xC8C2, 0x0080, _ar0231_ext_isp_delay_}, \
}

#define AP0200_EXP_CTL_REQ \
{ \
    { 0xC842, 0x01, _ar0231_ext_isp_delay_}, \
}

#define AP0200_GAMMA_COMMON \
{ \
    { 0xCA08, 0x0003, _ar0231_ext_isp_delay_ }, \
    { 0xCA34, 0x0064, _ar0231_ext_isp_delay_ }, \
}

#define AP0200_GAMMA_TABLE \
{ \
    { 0xBC0A, 0x0000, _ar0231_ext_isp_delay_ }, \
    { 0xBC0C, 0x000a, _ar0231_ext_isp_delay_ }, \
    { 0xBC0E, 0x000b, _ar0231_ext_isp_delay_ }, \
    { 0xBC10, 0x001a, _ar0231_ext_isp_delay_ }, \
    { 0xBC12, 0x0027, _ar0231_ext_isp_delay_ }, \
    { 0xBC14, 0x0032, _ar0231_ext_isp_delay_ }, \
    { 0xBC16, 0x003d, _ar0231_ext_isp_delay_ }, \
    { 0xBC18, 0x0046, _ar0231_ext_isp_delay_ }, \
    { 0xBC1A, 0x004f, _ar0231_ext_isp_delay_ }, \
    { 0xBC1C, 0x005f, _ar0231_ext_isp_delay_ }, \
    { 0xBC1E, 0x006d, _ar0231_ext_isp_delay_ }, \
    { 0xBC20, 0x007a, _ar0231_ext_isp_delay_ }, \
    { 0xBC22, 0x0087, _ar0231_ext_isp_delay_ }, \
    { 0xBC24, 0x009d, _ar0231_ext_isp_delay_ }, \
    { 0xBC26, 0x00b1, _ar0231_ext_isp_delay_ }, \
    { 0xBC28, 0x00c4, _ar0231_ext_isp_delay_ }, \
    { 0xBC2A, 0x00d6, _ar0231_ext_isp_delay_ }, \
    { 0xBC2C, 0x00f5, _ar0231_ext_isp_delay_ }, \
    { 0xBC2E, 0x0112, _ar0231_ext_isp_delay_ }, \
    { 0xBC30, 0x012d, _ar0231_ext_isp_delay_ }, \
    { 0xBC32, 0x0145, _ar0231_ext_isp_delay_ }, \
    { 0xBC34, 0x0172, _ar0231_ext_isp_delay_ }, \
    { 0xBC36, 0x019b, _ar0231_ext_isp_delay_ }, \
    { 0xBC38, 0x01c1, _ar0231_ext_isp_delay_ }, \
    { 0xBC3A, 0x01e3, _ar0231_ext_isp_delay_ }, \
    { 0xBC3C, 0x0223, _ar0231_ext_isp_delay_ }, \
    { 0xBC3E, 0x025d, _ar0231_ext_isp_delay_ }, \
    { 0xBC40, 0x0292, _ar0231_ext_isp_delay_ }, \
    { 0xBC42, 0x02c3, _ar0231_ext_isp_delay_ }, \
    { 0xBC44, 0x031d, _ar0231_ext_isp_delay_ }, \
    { 0xBC46, 0x036f, _ar0231_ext_isp_delay_ }, \
    { 0xBC48, 0x03b9, _ar0231_ext_isp_delay_ }, \
    { 0xBC4A, 0x03ff, _ar0231_ext_isp_delay_ }, \
    { 0xBC4C, 0x0000, _ar0231_ext_isp_delay_ }, \
    { 0xBC4E, 0x0002, _ar0231_ext_isp_delay_ }, \
    { 0xBC50, 0x0004, _ar0231_ext_isp_delay_ }, \
    { 0xBC52, 0x0007, _ar0231_ext_isp_delay_ }, \
    { 0xBC54, 0x0009, _ar0231_ext_isp_delay_ }, \
    { 0xBC56, 0x000b, _ar0231_ext_isp_delay_ }, \
    { 0xBC58, 0x000d, _ar0231_ext_isp_delay_ }, \
    { 0xBC5A, 0x000f, _ar0231_ext_isp_delay_ }, \
    { 0xBC5C, 0x0011, _ar0231_ext_isp_delay_ }, \
    { 0xBC5E, 0x0016, _ar0231_ext_isp_delay_ }, \
    { 0xBC60, 0x001a, _ar0231_ext_isp_delay_ }, \
    { 0xBC62, 0x001f, _ar0231_ext_isp_delay_ }, \
    { 0xBC64, 0x0023, _ar0231_ext_isp_delay_ }, \
    { 0xBC66, 0x002c, _ar0231_ext_isp_delay_ }, \
    { 0xBC68, 0x0034, _ar0231_ext_isp_delay_ }, \
    { 0xBC6A, 0x003d, _ar0231_ext_isp_delay_ }, \
    { 0xBC6C, 0x0046, _ar0231_ext_isp_delay_ }, \
    { 0xBC6E, 0x0057, _ar0231_ext_isp_delay_ }, \
    { 0xBC70, 0x0069, _ar0231_ext_isp_delay_ }, \
    { 0xBC72, 0x007a, _ar0231_ext_isp_delay_ }, \
    { 0xBC74, 0x008c, _ar0231_ext_isp_delay_ }, \
    { 0xBC76, 0x00af, _ar0231_ext_isp_delay_ }, \
    { 0xBC78, 0x00d2, _ar0231_ext_isp_delay_ }, \
    { 0xBC7A, 0x00f5, _ar0231_ext_isp_delay_ }, \
    { 0xBC7C, 0x0118, _ar0231_ext_isp_delay_ }, \
    { 0xBC7E, 0x015e, _ar0231_ext_isp_delay_ }, \
    { 0xBC80, 0x01a4, _ar0231_ext_isp_delay_ }, \
    { 0xBC82, 0x01ea, _ar0231_ext_isp_delay_ }, \
    { 0xBC84, 0x022f, _ar0231_ext_isp_delay_ }, \
    { 0xBC86, 0x02b4, _ar0231_ext_isp_delay_ }, \
    { 0xBC88, 0x032b, _ar0231_ext_isp_delay_ }, \
    { 0xBC8A, 0x0399, _ar0231_ext_isp_delay_ }, \
    { 0xBC8C, 0x03ff, _ar0231_ext_isp_delay_ }, \
}

#define AP0200_REFRESH_CMD        0x8606
#define AP0200_GET_STATE_CMD         0x8607

#define AP0200_CMD_DOORBELL_BIT        (1 << 15)
#define AP0200_EXP_CTL_BIT        (1 << 0)

#define AP0200_STATE_ENOERR        0
#define AP0200_STATE_EBUSY        9

#define AP0200_REFRESH_TIME_OUT        50     //millisecond
#define AP0200_DOOR_BELL_TIME_OUT        10    //millisecond
#define AP0200_EXP_CHG_TIME_OUT        100    //millisecond

#define FLOAT_TO_FIXEDPOINT(b, f) \
  (((f)*(1<<(b))))

static int ar0231_ext_isp_detect(max96712_context_t* ctxt, uint32 link);
static int ar0231_ext_isp_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg);
static int ar0231_ext_isp_init_link(max96712_context_t* ctxt, uint32 link);
static int ar0231_ext_isp_start_link(max96712_context_t* ctxt, uint32 link);
static int ar0231_ext_isp_stop_link(max96712_context_t* ctxt, uint32 link);
static int ap0200_poll_doorbell_bit(max96712_context_t* pCtxt, max96712_sensor_info_t* pSensor, unsigned int* reg_val);
static int ap0200_refresh_cmd(max96712_context_t* pCtxt, max96712_sensor_info_t* pSensor);
static int ap0200_req_exp_ctl(max96712_context_t* pCtxt, max96712_sensor_info_t* pSensor);
static int ap0200_apply_exposure(max96712_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info);
static int ap0200_apply_gamma(max96712_context_t* ctxt, uint32 link, qcarcam_gamma_config_t* gamma);
static int ar0231_ext_set_trigger_mode(max96712_context_t* ctxt, uint32 link);


static max96712_sensor_t ar0231_ext_isp_info = {
    .id = MAXIM_SENSOR_ID_AR0231_EXT_ISP,
    .detect = ar0231_ext_isp_detect,
    .get_link_cfg = ar0231_ext_isp_get_link_cfg,

    .init_link = ar0231_ext_isp_init_link,
    .start_link = ar0231_ext_isp_start_link,
    .stop_link = ar0231_ext_isp_stop_link,
    .apply_exposure = ap0200_apply_exposure,
    .apply_gamma = ap0200_apply_gamma
};

typedef struct
{
    struct camera_i2c_reg_setting cam_ser_reg_setting;
    struct camera_i2c_reg_setting ap0200_reg_setting_w;
    struct camera_i2c_reg_setting ap0200_reg_setting_b;
}ar0231_ext_isp_contxt_t;


static struct camera_i2c_reg_array max9295_init_8bit_reg[] = CAM_SER_INIT_8BIT;
static struct camera_i2c_reg_array max9295_init_10bit_reg[] = CAM_SER_INIT_10BIT;
static struct camera_i2c_reg_array max9295_start_reg[] = CAM_SER_START;
static struct camera_i2c_reg_array max9295_stop_reg[] = CAM_SER_STOP;

static struct camera_i2c_reg_array ap0200_sys_cmd_reg[] = AP0200_SYS_CMD;
static struct camera_i2c_reg_array ap0200_trigger_mode_reg[] = AP0200_TRIGGER_MODE;
static struct camera_i2c_reg_array ap0200_change_config_reg[] = AP0200_CHANGE_CONFIG;
static struct camera_i2c_reg_array ap0200_ae_mode_auto[] = AP0200_AE_MODE_AUTO;
static struct camera_i2c_reg_array ap0200_ae_mode_manual[] = AP0200_AE_MODE_MANUAL;
static struct camera_i2c_reg_array ap0200_exp_config[] = AP0200_EXP_CONFIG;
static struct camera_i2c_reg_array ap0200_exp_ctl_req[] = AP0200_EXP_CTL_REQ;
static struct camera_i2c_reg_array ap0200_gamma_common_config[] = AP0200_GAMMA_COMMON;
static struct camera_i2c_reg_array ap0200_gamma_table_config[] = AP0200_GAMMA_TABLE;

static maxim_pipeline_t ar0231_ext_isp_pipelines[AR0231_EXT_ISP_MODE_MAX][MAXIM_PIPELINE_MAX] =
{
    [AR0231_EXT_ISP_MODE_YUV8_CROP_METADATA] = {
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = QCARCAM_FMT_UYVY_8,
                .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT_CROP_META, .fps = 30.0f},
                .channel_info = {.vc = 0, .dt = CSI_DT_RAW8, .cid = CID_VC0},
                .crop_info = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT_CROP_META, .x_offset = SENSOR_X_OFFSET, .y_offset = SENSOR_Y_OFFSET},
            }
        }
    },
    [AR0231_EXT_ISP_MODE_YUV8_FULL_RES] = {
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = QCARCAM_FMT_UYVY_8,
                .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                .channel_info = {.vc = 0, .dt = CSI_DT_RAW8, .cid = CID_VC0},
            }
        }
    },
    [AR0231_EXT_ISP_MODE_YUV10_CROP_METADATA] = {
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = FMT_UYVY_MIPI10,
                .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT_CROP_META, .fps = 30.0f},
                .channel_info = {.vc = 0, .dt = CSI_DT_RAW10, .cid = CID_VC0},
                .crop_info = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT_CROP_META, .x_offset = SENSOR_X_OFFSET, .y_offset = SENSOR_Y_OFFSET},
            }
        }
    },
    [AR0231_EXT_ISP_MODE_YUV10_FULL_RES] = {
        {
            .id = MAXIM_PIPELINE_X,
            .mode =
            {
                .fmt = FMT_UYVY_MIPI10,
                .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                .channel_info = {.vc = 0, .dt = CSI_DT_RAW10, .cid = CID_VC0},
            }
        }
    }
};

max96712_sensor_t* ar0231_ext_isp_get_sensor_info(void)
{
    return &ar0231_ext_isp_info;
}

static int ar0231_create_ctxt(max96712_sensor_info_t* pSensor)
{
    int rc = 0;

    if (pSensor->pPrivCtxt)
    {
        SLOW("Ctxt already created");
        return 0;
    }

    if(pSensor->mode >= AR0231_EXT_ISP_MODE_MAX)
    {
        SERR("Unsupported sensor mode %d", pSensor->mode);
        return -1;
    }

    ar0231_ext_isp_contxt_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(ar0231_ext_isp_contxt_t));
    if (pCtxt)
    {
        memset(pCtxt, 0x0, sizeof(*pCtxt));

        pCtxt->cam_ser_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->cam_ser_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        pCtxt->ap0200_reg_setting_w.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->ap0200_reg_setting_w.data_type = CAMERA_I2C_WORD_DATA;

        pCtxt->ap0200_reg_setting_b.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->ap0200_reg_setting_b.data_type = CAMERA_I2C_BYTE_DATA;

        pSensor->pPrivCtxt = pCtxt;
    }
    else
    {
        SERR("Failed to allocate sensor context");
        rc = -1;
    }

    return rc;
}

static int ar0231_ext_isp_detect(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = (max96712_context_t*)ctxt;
    int rc = 0;
    int i = 0;
    // List of serializer addresses we support. Last address is left blank for alias address
    uint16 supported_ser_addr[] = {0xC4, 0x88, 0x80, 0};
    int num_addr = STD_ARRAY_SIZE(supported_ser_addr);
    struct camera_i2c_reg_array read_reg[1] = {};
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    sensor_platform_func_table_t* sensor_fcn_tbl = &pCtxt->platform_fcn_tbl;
    ar0231_ext_isp_contxt_t* ar0231_ctxt;
    boolean isSkipRemap = FALSE;

    rc = ar0231_create_ctxt(pSensor);
    if (rc)
    {
        SERR("Failed to create ctxt for link %d", link);
        return rc;
    }

    ar0231_ctxt = pSensor->pPrivCtxt;

    ar0231_ctxt->cam_ser_reg_setting.reg_array = read_reg;
    ar0231_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
    supported_ser_addr[num_addr-1] = pSensor->serializer_alias;

    /* Detect far end serializer */
    for (i = 0; i < num_addr; i++)
    {
        ar0231_ctxt->cam_ser_reg_setting.reg_array[0].reg_addr = MSM_SER_CHIP_ID_REG_ADDR;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, supported_ser_addr[i], &ar0231_ctxt->cam_ser_reg_setting);
        if (!rc)
        {
            pSensor->serializer_addr = supported_ser_addr[i];
            break;
        }
    }

    if (i == num_addr)
    {
        SENSOR_WARN("No Camera connected to Link %d of max96712 0x%x", link, pCtxt->slave_addr);
        return -1;
    }
    else if (pSensor->serializer_alias == pSensor->serializer_addr)
    {
        SENSOR_WARN("LINK %d already re-mapped 0x%x", link, pSensor->serializer_addr);

        // Read remote external ISP to check whether re-map can be skipped
        ar0231_ctxt->ap0200_reg_setting_w.reg_array = read_reg;
        ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(read_reg);
        ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_addr = 0x0000;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, pSensor->sensor_alias, &ar0231_ctxt->ap0200_reg_setting_w);
        if (rc || ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_data != CAM_EXT_ISP_CHIP_ID)
        {
            SENSOR_WARN("LINK %d failed to access sensor(0x%x), could not skip remap", link, pSensor->sensor_alias);
        }
        else
        {
            SENSOR_WARN("LINK %d remote ser 0x%x, sensor 0x%x accessed, could skip remap", link, pSensor->serializer_alias, pSensor->sensor_alias);
            isSkipRemap = TRUE;
        }

        rc = 0;
    }

    if (FALSE == isSkipRemap)
    {
        struct camera_i2c_reg_array remap_ser[] = {
            {0x0, pSensor->serializer_alias, _ar0231_ext_isp_delay_}
        };

        //link reset, remap cam, create broadcast addr,
        struct camera_i2c_reg_array remap_ser_2[] = {
            {0x0042, pSensor->sensor_alias, _ar0231_ext_isp_delay_},
            {0x0043, CAM_EXT_ISP_DEFAULT_ADDR, _ar0231_ext_isp_delay_},
            {0x0044, CAM_SER_BROADCAST_ADDR, _ar0231_ext_isp_delay_},
            {0x0045, pSensor->serializer_alias, _ar0231_ext_isp_delay_}
        };

        SENSOR_WARN("Detected Camera connected to Link %d, Ser ID[0x%x]: addr 0x%x",
            link, ar0231_ctxt->cam_ser_reg_setting.reg_array[0].reg_data, MSM_SER_CHIP_ID_REG_ADDR);

        ar0231_ctxt->cam_ser_reg_setting.reg_array = remap_ser;
        ar0231_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl,pSensor->serializer_addr, &ar0231_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to change serializer address (0x%x) of max96712 0x%x Link %d",
                pSensor->serializer_addr, pCtxt->slave_addr, link);
            return -1;
        }

        // Read mapped SER to double check if successful
        ar0231_ctxt->cam_ser_reg_setting.reg_array = read_reg;
        ar0231_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(read_reg);
        ar0231_ctxt->cam_ser_reg_setting.reg_array[0].reg_addr = 0x0000;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, pSensor->serializer_alias, &ar0231_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to read serializer(0x%x) after remap", pSensor->serializer_alias);
            return -1;
        }

        if (pSensor->serializer_alias != ar0231_ctxt->cam_ser_reg_setting.reg_array[0].reg_data)
        {
            SENSOR_WARN("Remote SER address remap failed: 0x%x, should be 0x%x",
                ar0231_ctxt->cam_ser_reg_setting.reg_array[0].reg_data, pSensor->serializer_alias);
            return -1;
        }

        ar0231_ctxt->cam_ser_reg_setting.reg_array = remap_ser_2;
        ar0231_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(remap_ser_2);
        rc = sensor_fcn_tbl->i2c_slave_write_array(pCtxt->ctrl, pSensor->serializer_alias, &ar0231_ctxt->cam_ser_reg_setting);
        if (rc)
        {
            SERR("Failed to reset link %d and remap cam on serializer(0x%x)", link, pSensor->serializer_alias);
            return -1;
        }

        //Read remote external ISP to check if successful
        ar0231_ctxt->ap0200_reg_setting_w.reg_array = read_reg;
        ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(read_reg);
        ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_addr = 0x0000;
        rc = sensor_fcn_tbl->i2c_slave_read(pCtxt->ctrl, pSensor->sensor_alias, &ar0231_ctxt->ap0200_reg_setting_w);
        if (rc || ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_data != CAM_EXT_ISP_CHIP_ID)
        {
            SENSOR_WARN("Failed to access correct ext isp (0x%x) chip-id (0x%x)",
                pSensor->sensor_alias, ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_data);
            return -1;
        }

        if (MAX96712_FSYNC_MODE_DISABLED != pCtxt->max96712_config.sensors[link].fsync_mode)
        {
            ar0231_ext_set_trigger_mode(ctxt, link);
        }
    }

    return rc;
}

static int ar0231_ext_isp_get_link_cfg(max96712_context_t* ctxt, uint32 link, max96712_link_cfg_t* p_cfg)
{
    int rc = 0;
    unsigned int mode;

    mode = ctxt->max96712_sensors[link].mode;

    if(mode < AR0231_EXT_ISP_MODE_MAX)
    {
        p_cfg->link_type = MAXIM_LINK_TYPE_PARALLEL;
        p_cfg->num_pipelines = 1;
        //only using pipeline X for now
        p_cfg->pipelines[0] = ar0231_ext_isp_pipelines[mode][MAXIM_PIPELINE_X];
    }
    else
    {
        p_cfg->num_pipelines = 0;
        SERR("Invalid sensor mode %d", mode);
        rc = -1;
    }
    return rc;
}

static int ar0231_ext_isp_init_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;
    int rc = 0;

    if (SENSOR_STATE_DETECTED == pSensor->state)
    {
        unsigned int mode;

        mode = ctxt->max96712_sensors[link].mode;
        switch (ar0231_ext_isp_pipelines[mode][0].mode.channel_info.dt)
        {
        case CSI_DT_RAW8:
        case CSI_DT_YUV422_8:
            ar0231_ctxt->cam_ser_reg_setting.reg_array = max9295_init_8bit_reg;
            ar0231_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_init_8bit_reg);
            break;
        case CSI_DT_RAW10:
        case CSI_DT_YUV422_10:
            ar0231_ctxt->cam_ser_reg_setting.reg_array = max9295_init_10bit_reg;
            ar0231_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_init_10bit_reg);
            break;
        default:
            SENSOR_WARN("link %d unknown DT: 0x%x", link,
                ar0231_ext_isp_pipelines[mode][link].mode.channel_info.dt);
            rc = -1;
        }

        if (!rc)
        {
            rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    pSensor->serializer_alias, &ar0231_ctxt->cam_ser_reg_setting);
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
        SERR("ar0231 %d init in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ar0231_ext_set_trigger_mode(max96712_context_t* pCtxt, uint32 link)
{
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;
    unsigned int state;
    unsigned int timeout = 10;
    int rc = 0;

    SHIGH("ar0231_ext_set_trigger_mode");

    //Trigger Mode
    ar0231_ctxt->ap0200_reg_setting_b.reg_array = ap0200_trigger_mode_reg;
    ar0231_ctxt->ap0200_reg_setting_b.size = STD_ARRAY_SIZE(ap0200_trigger_mode_reg);
    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
        pCtxt->ctrl,
        pSensor->sensor_alias,
        &ar0231_ctxt->ap0200_reg_setting_b);
    if (rc)
    {
        SERR("AP0200 0x%x failed to issue trigger mode", pSensor->sensor_alias);
    }
    else
    {
        //Change-config
        ar0231_ctxt->ap0200_reg_setting_w.reg_array = ap0200_change_config_reg;
        ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(ap0200_change_config_reg);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_w);
        if (rc)
        {
            SERR("AP0200 0x%x failed to issue change config command", pSensor->sensor_alias);
        }
        else
        {
            while(timeout--)
            {
                if (!(rc = ap0200_poll_doorbell_bit(pCtxt, pSensor, &state)))
                    break;
            }
            if (rc)
            {
                SERR("AP0200 0x%x doorbell poll failed", pSensor->sensor_alias);
            }
        }
    }

    return rc;
}

static int ar0231_ext_isp_start_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    int rc = 0;

    if (SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;

        SHIGH("starting serializer 0x%x", pSensor->serializer_alias);

        ar0231_ctxt->cam_ser_reg_setting.reg_array = max9295_start_reg;
        ar0231_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_start_reg);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &ar0231_ctxt->cam_ser_reg_setting);
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
        SERR("ar0231 %d start in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ar0231_ext_isp_stop_link(max96712_context_t* ctxt, uint32 link)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    int rc = 0;

    if (SENSOR_STATE_STREAMING == pSensor->state)
    {
        ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;

        SHIGH("stopping serializer 0x%x", pSensor->serializer_alias);

        ar0231_ctxt->cam_ser_reg_setting.reg_array = max9295_stop_reg;
        ar0231_ctxt->cam_ser_reg_setting.size = STD_ARRAY_SIZE(max9295_stop_reg);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &ar0231_ctxt->cam_ser_reg_setting)))
        {
            SERR("Failed to stop serializer(0x%x)", pSensor->serializer_alias);
        }

        pSensor->state = SENSOR_STATE_INITIALIZED;
    }
    else
    {
        SERR("ar0231 %d stop in wrong state %d", link, pSensor->state);
        rc = -1;
    }

    return rc;
}

//external ISP poll doorbell bit of system command register
static int ap0200_poll_doorbell_bit(max96712_context_t* pCtxt, max96712_sensor_info_t* pSensor, unsigned int* reg_val)
{
    int rc;
    unsigned int timeout = AP0200_DOOR_BELL_TIME_OUT;
    ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;

    ar0231_ctxt->ap0200_reg_setting_w.reg_array = ap0200_sys_cmd_reg;
    ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(ap0200_sys_cmd_reg);

    while (timeout)
    {
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_w)))
        {
            SENSOR_ERROR("Failed to read AP0200 0x%x command status", pSensor->sensor_alias);
            return rc;
        }

        if (!(ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_data & AP0200_CMD_DOORBELL_BIT))
        {
            //doorbell bit has been cleared, means command finished, returns result
            *reg_val = ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_data;
            rc = 0;
            break;
        }
        else
        {
            timeout--;
            CameraSleep(1);
            continue;
        }
    }

    if (0 == timeout)
    {
        SENSOR_ERROR("AP0200 0x%x poll door bell bit failed", pSensor->sensor_alias);
        rc = -1;
    }

    return rc;
}

//submit refresh command for external ISP, and waits for completion
static int ap0200_refresh_cmd(max96712_context_t* pCtxt, max96712_sensor_info_t* pSensor)
{
    ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;
    unsigned int timeout = AP0200_REFRESH_TIME_OUT;
    unsigned int state;
    int rc;

    //submit refresh command
    ar0231_ctxt->ap0200_reg_setting_w.reg_array = ap0200_sys_cmd_reg;
    ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(ap0200_sys_cmd_reg);
    ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_data = AP0200_REFRESH_CMD;

    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
        pCtxt->ctrl,
        pSensor->sensor_alias,
        &ar0231_ctxt->ap0200_reg_setting_w);

    if (rc || ap0200_poll_doorbell_bit(pCtxt, pSensor, &state)
        || state != AP0200_STATE_ENOERR)
    {
        SENSOR_ERROR("AP0200 0x%x failed to issue refresh command", pSensor->sensor_alias);
        return -1;
    }

    //retrieve status of last refresh command, loop until complete or time out
    while(timeout)
    {
        ar0231_ctxt->ap0200_reg_setting_w.reg_array = ap0200_sys_cmd_reg;
        ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(ap0200_sys_cmd_reg);
        ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_data = AP0200_GET_STATE_CMD;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_w);
        if (rc || ap0200_poll_doorbell_bit(pCtxt, pSensor, &state))
        {
            SENSOR_ERROR("AP0200 0x%x failed to get latest refresh status", pSensor->sensor_alias);
            return -1;
        }

        if (AP0200_STATE_ENOERR == state)
        {
            rc = 0;
            break;
        }
        else if (AP0200_STATE_EBUSY == state)
        {
            timeout--;
            CameraSleep(1);
            continue;
        }
        else
        {
            SENSOR_ERROR("AP0200 0x%x unknown refresh status", pSensor->sensor_alias);
            rc = -1;
            break;
        }
    }

    if (0 == timeout)
    {
        SENSOR_ERROR("AP0200 0x%x refresh command time out", pSensor->sensor_alias);
        rc = -1;
    }

    return rc;
}

static int ap0200_req_exp_ctl(max96712_context_t* pCtxt, max96712_sensor_info_t* pSensor)
{
    int rc;
    int timeout = AP0200_EXP_CHG_TIME_OUT;
    ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;

    //request sensor manager to submit a new exposure
    ar0231_ctxt->ap0200_reg_setting_b.reg_array = ap0200_exp_ctl_req;
    ar0231_ctxt->ap0200_reg_setting_b.size = STD_ARRAY_SIZE(ap0200_exp_ctl_req);
    ar0231_ctxt->ap0200_reg_setting_b.reg_array[0].reg_data = AP0200_EXP_CTL_BIT;

    rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
        pCtxt->ctrl,
        pSensor->sensor_alias,
        &ar0231_ctxt->ap0200_reg_setting_b);
    if (rc)
    {
        SENSOR_ERROR("ISP(0x%x) sensor manager commit new exposure failed", pSensor->sensor_alias);
        return rc;
    }

    //poll request bit cleared
    while (0 != timeout)
    {
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_b)))
        {
            SENSOR_ERROR("Failed to read ISP sensor ctrl register = 0x%x", pSensor->sensor_alias);
            return rc;
        }

        if (!(ar0231_ctxt->ap0200_reg_setting_b.reg_array[0].reg_data & AP0200_EXP_CTL_BIT))
        {
            //exposure bit has been cleared, break;
            rc = 0;
            break;
        }
        else
        {
            timeout--;
            CameraSleep(1);
            continue;
        }
    }

    if (0 == timeout)
    {
        rc = -1;
        SENSOR_ERROR("ISP(0x%x) submit exposure change time out", pSensor->sensor_alias);
    }

    return rc;
}

static int ap0200_apply_exposure(max96712_context_t* ctxt, uint32 link, sensor_exposure_info_t* exposure_info)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;
    int rc = 0;

    if (SENSOR_STATE_STREAMING != pSensor->state)
    {
        SERR("ar0231 %d apply exposure in wrong state %d", link, pSensor->state);
        rc = -1;
    }
    else if(exposure_info->exposure_mode_type == QCARCAM_EXPOSURE_AUTO)
    {
        //switch to auto mode
        ar0231_ctxt->ap0200_reg_setting_b.reg_array = ap0200_ae_mode_auto;
        ar0231_ctxt->ap0200_reg_setting_b.size = STD_ARRAY_SIZE(ap0200_ae_mode_auto);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_b);
        if (rc)
        {
            SERR("AP0200 0x%x failed to switch to auto AE mode", pSensor->sensor_alias);
            return rc;
        }

        //switching AE mode requires a refresh command
        rc = ap0200_refresh_cmd(pCtxt, pSensor);
        if (rc)
        {
            SERR("AP0200 0x%x refresh cmd failed", pSensor->sensor_alias);
            return rc;
        }
    }
    else if (exposure_info->exposure_mode_type == QCARCAM_EXPOSURE_MANUAL)
    {
        //switch to manual mode
        ar0231_ctxt->ap0200_reg_setting_b.reg_array = ap0200_ae_mode_manual;
        ar0231_ctxt->ap0200_reg_setting_b.size = STD_ARRAY_SIZE(ap0200_ae_mode_manual);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_b);
        if (rc)
        {
            SERR("AP0200 0x%x failed to switch to manual AE mode", pSensor->sensor_alias);
            return rc;
        }

        //switching AE mode requires a refresh command
        rc = ap0200_refresh_cmd(pCtxt, pSensor);
        if (rc)
        {
            SERR("AP0200 0x%x refresh cmd failed", pSensor->sensor_alias);
            return rc;
        }

        //configure exposure time and gain
        //register value is unsigned fixed-point with 7 fractional bits
        ar0231_ctxt->ap0200_reg_setting_w.reg_array = ap0200_exp_config;
        ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(ap0200_exp_config);
        ar0231_ctxt->ap0200_reg_setting_w.reg_array[0].reg_data = STD_MIN(FLOAT_TO_FIXEDPOINT(7, exposure_info->exposure_time), 0xffff);
        ar0231_ctxt->ap0200_reg_setting_w.reg_array[1].reg_data = STD_MIN(FLOAT_TO_FIXEDPOINT(7, exposure_info->real_gain), 0xffff);

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_w);
        if (rc)
        {
            SERR("AP0200 0x%x failed to configure exposure time and gain", pSensor->sensor_alias);
            return rc;
        }

        //configure exposure needs to submit a exposure change to sensor manager
        rc = ap0200_req_exp_ctl(pCtxt, pSensor);
        if (rc)
        {
            SERR("AP0200 0x%x failed to apply exposure change", pSensor->sensor_alias);
            return rc;
        }
    }
    else
    {
        //TODO: pass-through for now
        SERR("not implemented AEC mode %d", exposure_info->exposure_mode_type);
        rc = 0;
    }

    return rc;
}

static int ap0200_apply_gamma(max96712_context_t* ctxt, uint32 link, qcarcam_gamma_config_t* gamma_info)
{
    max96712_context_t* pCtxt = ctxt;
    max96712_sensor_info_t* pSensor = &pCtxt->max96712_sensors[link];
    ar0231_ext_isp_contxt_t* ar0231_ctxt = pSensor->pPrivCtxt;
    int rc = 0;

    if (SENSOR_STATE_STREAMING != pSensor->state)
    {
        SERR("ar0231 %d apply gamma in wrong state %d", link, pSensor->state);
        rc = -1;
    }
    else if(gamma_info->config_type == QCARCAM_GAMMA_EXPONENT)
    {
        unsigned int reg_value = gamma_info->gamma.f_value * 100;

        ar0231_ctxt->ap0200_reg_setting_w.reg_array = ap0200_gamma_common_config;
        ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(ap0200_gamma_common_config);
        ar0231_ctxt->ap0200_reg_setting_w.reg_array[1].reg_data = reg_value & 0xffff;

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_w);
        if (rc)
        {
            SERR("external ISP 0x%x failed to configure gamma exponent", pSensor->sensor_alias);
            return rc;
        }
    }
    else if (gamma_info->config_type == QCARCAM_GAMMA_KNEEPOINTS)
    {
        unsigned int table_len = gamma_info->gamma.table.length;
        unsigned int *p_value = gamma_info->gamma.table.p_value;
        unsigned int idx = 0;

        if (table_len != STD_ARRAY_SIZE(ap0200_gamma_table_config))
        {
            SERR("gamma table to be configured for AP0200 0x%x is invalid", pSensor->sensor_alias);
            return -1;
        }

        ar0231_ctxt->ap0200_reg_setting_w.reg_array = ap0200_gamma_table_config;
        ar0231_ctxt->ap0200_reg_setting_w.size = STD_ARRAY_SIZE(ap0200_gamma_table_config);

        for (; idx < table_len; ++idx)
        {
            ar0231_ctxt->ap0200_reg_setting_w.reg_array[idx].reg_data = p_value[idx];
        }

        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ar0231_ctxt->ap0200_reg_setting_w);
        if (rc)
        {
            SERR("AP0200 0x%x failed to configure gamma table", pSensor->sensor_alias);
            return rc;
        }
    }
    else
    {
        SERR("unsupported gamma configure type for AP0200 0x%x", pSensor->sensor_alias);
        return -1;
    }

    return rc;
}

