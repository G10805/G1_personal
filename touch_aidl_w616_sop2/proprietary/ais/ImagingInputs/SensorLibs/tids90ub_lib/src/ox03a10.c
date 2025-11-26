/**
 * @file ox03a10.c
 *
 * Copyright (c) 2019-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ox03a10.h"
#include <string.h>

#define ADD_I2C_REG_ARRAY(_a_, _size_, _addr_, _val_, _delay_) \
do { \
    _a_[_size_].reg_addr = _addr_; \
    _a_[_size_].reg_data = _val_; \
    _a_[_size_].delay = _delay_; \
    _size_++; \
} while(0)

/*GPIO2*/
#define PWR_GPIO_UP          0x9
#define PWR_GPIO_DOWN        0x8
#define PWR_GPIO_UP_DELAY    50000 //from spec 50ms+
#define PWR_GPIO_DOWN_DELAY  1000

/*EXPOSURE*/
#define AGAIN_RANGE_COUNT     4
#define AGAIN_RANGE_STEP      0.0625f
#define DCG_GAIN              (16.0f / 12.9f)

#define INIT_SER \
{ \
    { 0x0E, 0xF0, _tids90ub_delay_ }, /*set all GPIO as output*/\
    { 0x0D, 0x0, PWR_GPIO_DOWN_DELAY }, /*Disable remote GPIO */ \
    { 0x0D, 0x4, PWR_GPIO_UP_DELAY }, /*Power up GPIO2*/\
}

#define INIT_DESER \
{ \
    { 0x58, 0x5e, _tids90ub_delay_ }, \
    { 0x5B, 0x30, _tids90ub_delay_}, \
    { 0x5d, SENSOR_DEFAULT_ADDR, _tids90ub_delay_ }, \
}

typedef struct
{
    float exposure_time;
    float real_gain;
    unsigned int line_count;
    unsigned int reg_dig_gain;
    unsigned int reg_analog_gain;
}ox03a10_hdr_exp_t;

typedef enum
{
    OX03A10_MODE_DEFAULT = 0,
    OX03A10_MODE_DCG12 = OX03A10_MODE_DEFAULT, /*DCG16, 12bit compressed*/
    OX03A10_MODE_DCG16 = 1, /*DCG16*/
    OX03A10_MODE_DCG16_VS12 = 2, /*DCG16+VS12*/
    OX03A10_MODE_MAX = 3
}ox03a10_mode_id_t;

typedef struct
{
    img_src_mode_t sources[MAX_PORT_SOURCES];
    unsigned int num_sources;

    struct camera_i2c_reg_array* init_array;
    uint32_t         size_init_array;

    uint64_t         pclk;
    uint32_t         line_length_pck;
    uint32_t         frame_length_lines;

}ox03a10_mode_t;

static int ox03a10_detect(tids90ub_context_t* ctxt, uint32 port);
static int ox03a10_get_port_cfg(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* p_cfg);
static int ox03a10_init_port(tids90ub_context_t* ctxt, uint32 port);
static int ox03a10_start_port(tids90ub_context_t* ctxt, uint32 port);
static int ox03a10_stop_port(tids90ub_context_t* ctxt, uint32 port);
static int ox03a10_apply_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info);
static int ox03a10_apply_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure);


static struct camera_i2c_reg_array ox03a10_tids90ub_953_init[] = INIT_SER;
static struct camera_i2c_reg_array ox03a10_tids90ub_init[] = INIT_DESER;
static struct camera_i2c_reg_array ox03a10_init_dcg12[] = INIT_OX03A10_DCG12;
static struct camera_i2c_reg_array ox03a10_init_dcg16[] = INIT_OX03A10_DCG16;
static struct camera_i2c_reg_array ox03a10_init_dcg16_vs12[] = INIT_OX03A10_DCG16_VS12;
static struct camera_i2c_reg_array ox03a10_start[] = START_OX03A10;
static struct camera_i2c_reg_array ox03a10_stop[] = STOP_OX03A10;

static ox03a10_mode_t ox03a10_modes[OX03A10_MODE_MAX] =
{
        [OX03A10_MODE_DCG12] =
        {
                .sources[0] =
                {
                       .fmt = QCARCAM_COLOR_FMT(QCARCAM_BAYER_BGGR, QCARCAM_BITDEPTH_12, QCARCAM_PACK_MIPI),
                       .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                       .channel_info = {.vc = 0x0, .dt = CSI_DT_RAW12, .cid = 0,},
                },
                .num_sources = 1,
                .init_array = ox03a10_init_dcg12,
                .size_init_array = STD_ARRAY_SIZE(ox03a10_init_dcg12),
                /*TO DO: initialize pclk/line_length_pck/frame_length_lines*/
                .pclk = 0,
                .line_length_pck = 0,
                .frame_length_lines = 0
        },
        [OX03A10_MODE_DCG16] =
        {
                .sources[0] =
                {
                       .fmt = QCARCAM_COLOR_FMT(QCARCAM_BAYER_BGGR, QCARCAM_BITDEPTH_16, QCARCAM_PACK_MIPI),
                       .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                       .channel_info = {.vc = 0x0, .dt = CSI_DT_RAW16, .cid = 0,},
                },
                .num_sources = 1,
                .init_array = ox03a10_init_dcg16,
                .size_init_array = STD_ARRAY_SIZE(ox03a10_init_dcg16),
                /*TO DO: initialize pclk/line_length_pck/frame_length_lines*/
                .pclk = 0,
                .line_length_pck = 0,
                .frame_length_lines = 0
        },
        [OX03A10_MODE_DCG16_VS12] =
        {
                 .sources[0] =
                 {
                       .fmt = QCARCAM_COLOR_FMT(QCARCAM_BAYER_BGGR, QCARCAM_BITDEPTH_16, QCARCAM_PACK_MIPI),
                       .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                       .channel_info = {.vc = 0x0, .dt = CSI_DT_RAW16, .cid = 0,},
                 },
                 .sources[1] =
                 {
                       .fmt = QCARCAM_COLOR_FMT(QCARCAM_BAYER_BGGR, QCARCAM_BITDEPTH_12, QCARCAM_PACK_MIPI),
                       .res = {.width = SENSOR_WIDTH, .height = SENSOR_HEIGHT, .fps = 30.0f},
                       .channel_info = {.vc = 0x1, .dt = CSI_DT_RAW12, .cid = 0,},
                 },
                .num_sources = 2,
                .init_array = ox03a10_init_dcg16_vs12,
                .size_init_array = STD_ARRAY_SIZE(ox03a10_init_dcg16_vs12),
                /*TO DO: initialize pclk/line_length_pck/frame_length_lines*/
                .pclk = 0,
                .line_length_pck = 0,
                .frame_length_lines = 0
        },
};

static const tids90ub_sensor_t ox03a10_info = {
    .detect = ox03a10_detect,
    .get_port_cfg = ox03a10_get_port_cfg,

    .init_port = ox03a10_init_port,
    .start_port = ox03a10_start_port,
    .stop_port = ox03a10_stop_port,
    .apply_exposure = ox03a10_apply_exposure,
    .apply_hdr_exposure = ox03a10_apply_hdr_exposure
};

typedef struct
{
    struct camera_i2c_reg_setting tids90ub_953_reg_setting;
    struct camera_i2c_reg_setting ox03a10_reg_setting;
}ox03a10_contxt_t;


const tids90ub_sensor_t* ox03a10_get_sensor_info(void)
{
    return &ox03a10_info;
}

static int ox03a10_create_ctxt(tids90ub_sensor_info_t* pSensor)
{
    int rc = 0;

    if (pSensor->pPrivCtxt)
    {
        SLOW("Ctxt already created");
        return 0;
    }

    ox03a10_contxt_t* pCtxt = CameraAllocate(CAMERA_ALLOCATE_ID_SENSOR_DRIVER_CTXT, sizeof(ox03a10_contxt_t));
    if (pCtxt)
    {
        memset(pCtxt, 0x0, sizeof(*pCtxt));

        pCtxt->tids90ub_953_reg_setting.reg_array = ox03a10_tids90ub_953_init;
        pCtxt->tids90ub_953_reg_setting.size = STD_ARRAY_SIZE(ox03a10_tids90ub_953_init);
        pCtxt->tids90ub_953_reg_setting.addr_type = CAMERA_I2C_BYTE_ADDR;
        pCtxt->tids90ub_953_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        pCtxt->ox03a10_reg_setting.reg_array = NULL;
        pCtxt->ox03a10_reg_setting.size = 0;
        pCtxt->ox03a10_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        pCtxt->ox03a10_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;

        pSensor->pPrivCtxt = pCtxt;
    }
    else
    {
        SERR("Failed to allocate sensor context");
        rc = -1;
    }

    return rc;
}


static int ox03a10_detect(tids90ub_context_t* pCtxt, uint32 port)
{
    int rc = 0;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    struct camera_i2c_reg_array read_reg[] = {{0, 0, 0}};
    ox03a10_contxt_t* ox03a10_ctxt;

    rc = ox03a10_create_ctxt(pSensor);
    if (rc)
    {
        SERR("Failed to create ctxt for link %d", port);
        return rc;
    }

    ox03a10_ctxt = pSensor->pPrivCtxt;

    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->serializer_alias,
            &ox03a10_ctxt->tids90ub_953_reg_setting))) {
        SERR("OX03A10[%d] Serializer (0x%x) unable to config",
                port, pSensor->serializer_alias);
        return -1;
    }

    /*read chip id*/
    ox03a10_ctxt->ox03a10_reg_setting.reg_array = read_reg;
    ox03a10_ctxt->ox03a10_reg_setting.size = STD_ARRAY_SIZE(read_reg);

    ox03a10_ctxt->ox03a10_reg_setting.reg_array[0].delay = 0;
    ox03a10_ctxt->ox03a10_reg_setting.reg_array[0].reg_data = 0;
    ox03a10_ctxt->ox03a10_reg_setting.reg_array[0].reg_addr = SLAVE_IDENT_PID_REG;
    if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_read(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ox03a10_ctxt->ox03a10_reg_setting))) {
        SERR("OX03A10[%d] unable to read ID (0x%x)", port, pSensor->sensor_alias);
        rc = -1;
    }
    else if (SLAVE_IDENT_PID_ID != ox03a10_ctxt->ox03a10_reg_setting.reg_array[0].reg_data)
    {
        SERR("OX03A10[%d] read back WRONG ID 0x%x [expecting 0x%x]",
                port,
                ox03a10_ctxt->ox03a10_reg_setting.reg_array[0].reg_data,
                SLAVE_IDENT_PID_ID);
        rc = -1;
    }
    else
    {
        SWARN("OX03A10[%d] detect OK", port);
        pSensor->state = TIDS90UB_SENSOR_STATE_DETECTED;
    }

    return rc;
}

static int ox03a10_get_port_cfg(tids90ub_context_t* ctxt, uint32 port, tids90ub_port_cfg_t* p_cfg)
{
    if (!ctxt || !p_cfg)
    {
        SERR("invalid tids90ub ctxt or port config.");
        return -1;
    }

    unsigned int sensor_mode = ctxt->config.sensors[port].sensor_mode;

    if (sensor_mode >= OX03A10_MODE_MAX)
    {
        SERR("invalid ox03a10 sensor mode %u.", sensor_mode);
        return -1;
    }

    p_cfg->num_sources = ox03a10_modes[sensor_mode].num_sources;

    for (int idx = 0; idx < p_cfg->num_sources; idx++)
    {
        p_cfg->sources[idx] = ox03a10_modes[sensor_mode].sources[idx];
        if (sensor_mode == OX03A10_MODE_DCG12
            || sensor_mode == OX03A10_MODE_DCG16)
        {
             p_cfg->sources[idx].channel_info.vc = port;
        }
    }

    p_cfg->deser_config = ox03a10_tids90ub_init;
    p_cfg->deser_config_size = STD_ARRAY_SIZE(ox03a10_tids90ub_init);

    return 0;
}

static int ox03a10_init_port(tids90ub_context_t* ctxt, uint32 port)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = NULL;
    uint32 sensor_addr = 0;
    unsigned int sensor_mode = ctxt->config.sensors[port].sensor_mode;

    if (TIDS90UB_PORT_MAX == port)
    {
        sensor_mode = ctxt->config.sensors[0].sensor_mode;
        sensor_addr = pCtxt->slave_alias_group;
    }
    else if (TIDS90UB_PORT_MAX < port)
    {
        SERR("OX03A10 invalid port %d", port);
        rc = -1;
    }
    else
    {
        pSensor = &pCtxt->sensors[port];
        sensor_addr = pSensor->sensor_alias;
        if (TIDS90UB_SENSOR_STATE_DETECTED != pSensor->state)
        {
            SERR("OX03A10[%d] init in wrong state %d", port, pSensor->state);
            rc = -1;
        }

    }

    if (sensor_mode >= OX03A10_MODE_MAX)
    {
        SERR("invalid ox03a10 sensor mode %u.", sensor_mode);
        rc = -1;
    }

    if (rc == 0)
    {
        struct camera_i2c_reg_setting ox03a10_reg_setting = {};

        SWARN("OX03A10[%d] (0x%x) init...", port, sensor_addr);

        ox03a10_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
        ox03a10_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;
        ox03a10_reg_setting.reg_array = ox03a10_modes[sensor_mode].init_array;
        ox03a10_reg_setting.size = ox03a10_modes[sensor_mode].size_init_array;
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
                    pCtxt->ctrl,
                    sensor_addr,
                    &ox03a10_reg_setting);
        if (rc)
        {
            SERR("OX03A10[%d] (0x%x) Failed to init", port, sensor_addr);
            rc = -1;
        }
        else if (pSensor)
        {
            pSensor->state = TIDS90UB_SENSOR_STATE_INITIALIZED;
        }
        else
        {
            for (uint32 i = 0; i < pCtxt->num_supported_sensors; i ++)
            {
                if (TIDS90UB_SENSOR_STATE_DETECTED == pCtxt->sensors[i].state)
                {
                    pCtxt->sensors[i].state = TIDS90UB_SENSOR_STATE_INITIALIZED;
                }
            }
        }
    }

    return rc;
}

static int ox03a10_start_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc;

    if (TIDS90UB_SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        ox03a10_contxt_t* ox03a10_ctxt = pSensor->pPrivCtxt;

        SHIGH("OX03A10[%d] starting", port);

        ox03a10_ctxt->ox03a10_reg_setting.reg_array = ox03a10_start;
        ox03a10_ctxt->ox03a10_reg_setting.size = STD_ARRAY_SIZE(ox03a10_start);
        rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ox03a10_ctxt->ox03a10_reg_setting);
        if (rc)
        {
            SERR("OX03A10[%d] failed to start", port);
        }
    }
    else
    {
        SERR("OX03A10[%d] start in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static int ox03a10_stop_port(tids90ub_context_t* ctxt, uint32 port)
{
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];
    int rc;

    if (TIDS90UB_SENSOR_STATE_STREAMING == pSensor->state)
    {
        ox03a10_contxt_t* ox03a10_ctxt = pSensor->pPrivCtxt;

        SHIGH("OX03A10[%d] stopping", port);

        ox03a10_ctxt->ox03a10_reg_setting.reg_array = ox03a10_stop;
        ox03a10_ctxt->ox03a10_reg_setting.size = STD_ARRAY_SIZE(ox03a10_stop);
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ox03a10_ctxt->ox03a10_reg_setting)))
        {
            SERR("OX03A10[%d] Failed to stop sensor", port);
        }
    }
    else
    {
        SERR("OX03A10[%d] stop in wrong state %d", port, pSensor->state);
        rc = -1;
    }

    return rc;
}

static unsigned int calc_reg_analog_gain (float real_gain, unsigned int *reg_digital_gain)
{
    unsigned int register_gain = 0x10;
    unsigned int step_count = 0;
    unsigned int i, again_region;
    float real_reg_gain = 0.0f;
    float digital_gain, normalised_again_range_step;

    real_gain = STD_MIN(MAX_SENSOR_TOTAL_GAIN, STD_MAX(1.0f, real_gain));

    for (i = 0; i < AGAIN_RANGE_COUNT; i ++)
    {
        again_region = (1 << i);
        if (real_gain < (again_region << 1) )
        {
            normalised_again_range_step = AGAIN_RANGE_STEP * again_region;
            step_count = (real_gain - again_region) / normalised_again_range_step;
            register_gain = (register_gain | step_count) << i;
            real_reg_gain = again_region + (float)step_count * normalised_again_range_step;
            digital_gain = real_gain / real_reg_gain;
            *reg_digital_gain = (unsigned int)digital_gain;
            *reg_digital_gain = (*reg_digital_gain << 10) | (unsigned int)((digital_gain - *reg_digital_gain) * 1024);
            break;
        }
    }

    if (real_reg_gain == 0.0f)
    {
        register_gain = 0xF8; //1111 1000 (template 1xxx.x000) => analog gain 15.5
        digital_gain = real_gain / MAX_SENSOR_ANALOG_GAIN;
        *reg_digital_gain = (unsigned int)digital_gain;
        *reg_digital_gain = (*reg_digital_gain << 10) | (unsigned int)((digital_gain - *reg_digital_gain) * 1024);
    }

    return register_gain;
}

static int ox03a10_apply_exposure(tids90ub_context_t* ctxt, uint32 port, sensor_exposure_info_t* exposure_info)
{
    int rc = 0;

    CAM_UNUSED(ctxt);
    CAM_UNUSED(port);
    CAM_UNUSED(exposure_info);

    return rc;
}

static int ox03a10_apply_hdr_exposure(tids90ub_context_t* ctxt, uint32 port, qcarcam_hdr_exposure_config_t* hdr_exposure)
{
    int rc = 0;
    tids90ub_context_t* pCtxt = ctxt;
    tids90ub_sensor_info_t* pSensor = &pCtxt->sensors[port];

    if (TIDS90UB_SENSOR_STATE_INITIALIZED == pSensor->state)
    {
        SERR("ox03a10 is in wrong state %d", pSensor->state);
        rc = -1;
    }
    else if(hdr_exposure->exposure_mode_type == QCARCAM_EXPOSURE_MANUAL ||
       hdr_exposure->exposure_mode_type == QCARCAM_EXPOSURE_AUTO)
    {
        ox03a10_contxt_t* ox03a10_ctxt = pSensor->pPrivCtxt;
        static struct camera_i2c_reg_array ox03a10_exp_reg_array[20];
        float long_adj_gain;
        ox03a10_hdr_exp_t hdr_exp[2];
        uint32 exp_size = 0;

        hdr_exp[1].exposure_time = hdr_exposure->exposure_time[1];
        hdr_exp[1].line_count = STD_MAX(1, STD_MIN(SENSOR_MODE_MAX_LINES, 1000 * 1000 * hdr_exp[1].exposure_time / RAW_TIME_NS));
        hdr_exp[1].real_gain = hdr_exposure->gain[1];
        hdr_exp[1].reg_analog_gain =
            calc_reg_analog_gain (hdr_exp[1].real_gain, &hdr_exp[1].reg_dig_gain);

        long_adj_gain = DCG_GAIN * hdr_exposure->gain[1];
        hdr_exp[0].exposure_time = hdr_exp[1].exposure_time;
        hdr_exp[0].line_count = hdr_exp[1].line_count;
        hdr_exp[0].real_gain = long_adj_gain;
        hdr_exp[0].reg_analog_gain =
            calc_reg_analog_gain (hdr_exp[0].real_gain, &hdr_exp[0].reg_dig_gain);

        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, GROUP_HOLD_REG, GROUP_0_HOLD_START, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_HCG_TOP_1, (hdr_exp[0].line_count & 0xff00) >> 8, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_HCG_TOP_2, (hdr_exp[0].line_count & 0x00ff) , _ox03a10_delay_);

        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_HCG_TOP_8, (hdr_exp[0].reg_analog_gain & 0xf0) >> 4, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_HCG_TOP_9, (hdr_exp[0].reg_analog_gain & 0x0f) << 4, _ox03a10_delay_);

        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_HCG_TOP_A, (hdr_exp[0].reg_dig_gain & 0x3c00) >> 10, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_HCG_TOP_B, (hdr_exp[0].reg_dig_gain & 0x03fc) >> 2, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_HCG_TOP_C, (hdr_exp[0].reg_dig_gain & 0x0003) << 6, _ox03a10_delay_);

        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_LCG_TOP_8, (hdr_exp[1].reg_analog_gain & 0xf0) >> 4, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_LCG_TOP_9, (hdr_exp[1].reg_analog_gain & 0x0f) << 4, _ox03a10_delay_);

        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_LCG_TOP_A, (hdr_exp[1].reg_dig_gain & 0x3c00) >> 10, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_LCG_TOP_B, (hdr_exp[1].reg_dig_gain & 0x03fc) >> 2, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, AEC_PK_CORE_LCG_TOP_C, (hdr_exp[1].reg_dig_gain & 0x0003) << 6, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, GROUP_HOLD_REG, GROUP_0_HOLD_END, _ox03a10_delay_);
        ADD_I2C_REG_ARRAY(ox03a10_exp_reg_array, exp_size, GROUP_HOLD_REG, GROUP_0_DELAYED_LAUNCH, _ox03a10_delay_);

        ox03a10_ctxt->ox03a10_reg_setting.reg_array = ox03a10_exp_reg_array;
        ox03a10_ctxt->ox03a10_reg_setting.size = exp_size;
        if ((rc = pCtxt->platform_fcn_tbl.i2c_slave_write_array(
            pCtxt->ctrl,
            pSensor->sensor_alias,
            &ox03a10_ctxt->ox03a10_reg_setting)))
        {
            SERR("OX03A10[%d] Failed to write exposure and gain to sensor", port);
        }
    }
    else
    {
        SERR("unsupported exposure mode type: %d", hdr_exposure->exposure_mode_type);
        rc = -1;
    }

    return rc;
}
